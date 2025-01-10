/*
	[ MkCht ]
	Copyright Regan "CKDEV" Green 2023-2025
	
	- MkCht.cpp -
	Chart generator
*/

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <unordered_set>
#include <cstdint>

#include "json.hpp"

// Exception types
class RuntimeError : public std::runtime_error
{
	public:
		RuntimeError(std::string what_arg = "") : std::runtime_error(what_arg) {}
};

// Chart class
enum SectionType : uint32_t
{
	MustHit = (1 << 0)
};

struct Section
{
	double time = 0;
	double length = 0;
	uint32_t type = 0;
};

enum NoteType : uint32_t
{
	// Direction
	Left = 0,
	Down = 1,
	Up = 2,
	Right = 3,
	Direction = Left | Down | Up | Right,

	// Flags
	Opponent = (1 << 2),
	AltAnim = (1 << 3),

	// Status
	Miss = (1 << 30),
	Hit = (1UL << 31),
	Status = Miss | Hit
};

struct Note
{
	double time = 0;
	double length = 0;
	uint32_t type = 0;

	friend bool operator==(const Note &lhs, const Note &rhs) { return lhs.time == rhs.time && lhs.length == rhs.length && lhs.type == rhs.type; }
};

class NoteHash
{
	public:
		size_t operator()(const Note &t) const
		{
			return (size_t)t.time;
		}
};

class Single
{
	public:
		double scroll = 0.0;
		std::vector<Section> sections;
		std::vector<Note> notes;

	public:
		void Out(std::ostream &stream)
		{
			stream << scroll << "," << std::endl;
			stream << sections.size() << "," << std::endl;
			stream << "(::PlayState::Section[]) {" << std::endl;
			for (auto &i : sections)
				stream << "{ " <<
				(double)i.time / 1000.0 << ", " <<
				(double)i.length / 1000.0 << ", " <<
				i.type << " }," << std::endl;
			stream << "}," << std::endl;
			stream << notes.size() << "," << std::endl;
			stream << "(::PlayState::Note[]) {" << std::endl;
			for (auto &i : notes)
				stream << "{ " <<
				(double)i.time / 1000.0 << ", " <<
				(double)i.length / 1000.0 << ", " <<
				i.type << " }," << std::endl;
			stream << "}," << std::endl;
		}
};

class Chart
{
	private:
		// Chart info
		std::vector<Single> singles;

	public:
		Chart(std::string name)
		{
			// Read JSON
			nlohmann::json json;
			{
				std::ifstream stream(name);
				if (!stream.is_open())
					throw RuntimeError(std::string("Failed to open ") + name);
				stream >> json;
			}

			// Read song info
			auto song_info = json["song"];

			auto song_notes = song_info["notes"];
			auto song_speeds = song_info["speed"];

			int single_i = 0;
			for (auto &single_data : song_notes)
			{
				Single single;

				double song_bpm = song_info["bpm"];
				double song_crochet = (60.0 * 1000.0 / 4.0) / song_bpm;

				double song_speed = song_speeds[single_i];
				single.scroll = ((song_speed * 0.45) * 1000.0) * 200.0 / 720.0;
				// * 0.45 is the base scroll speed
				// * 1000 for pixels scrolled in a second
				// * 200 / 720 is to convert the screenspace

				// Read chart
				std::unordered_set<Note, NoteHash> note_fudge;
				double milli = 0.0;

				for (auto &section : single_data)
				{
					// Read section
					Section new_section;
					new_section.time = milli;

					bool section_must_hit = section["mustHitSection"] == true;
					bool section_alt_anim = section["altAnim"] == true;

					if (section_must_hit)
						new_section.type |= SectionType::MustHit;

					if (section["changeBPM"] == true)
					{
						song_bpm = section["bpm"];
						song_crochet = (60.0 * 1000.0 / 4.0) / song_bpm;
					}

					new_section.length = song_crochet * 16.0;
					milli += song_crochet * 16.0;

					for (auto &note : section["sectionNotes"])
					{
						// Read note
						Note new_note;
						new_note.time = note[0];
						new_note.length = std::floor((double)note[2] / song_crochet) * song_crochet;

						new_note.type = NoteType((int)note[1] & (NoteType::Direction | NoteType::Opponent));
						if (!section_must_hit)
							new_note.type ^= NoteType::Opponent;
						if (note[3] == true)
							new_note.type |= NoteType::AltAnim;
						if ((new_note.type & NoteType::Opponent) && section_alt_anim)
							new_note.type |= NoteType::AltAnim;

						// Check fudge
						auto fudge_find = note_fudge.find(new_note);
						if (fudge_find != note_fudge.end())
							continue;

						// Push note
						note_fudge.emplace(new_note);
						single.notes.push_back(new_note);
					}

					// Push section
					single.sections.push_back(new_section);
				}

				// Sort notes
				std::sort(single.notes.begin(), single.notes.end(), [](Note a, Note b) {
					if (a.time == b.time)
						return a.type < b.type; // Same time, sort by type
					else
						return a.time < b.time; // Sort by time
				});

				// Push single
				singles.push_back(single);
				single_i++;
			}
		}

		~Chart()
		{

		}

		void Out(std::ostream &stream)
		{
			for (auto &i : singles)
			{
				stream << "{" << std::endl;
				i.Out(stream);
				stream << "}";
				if (&i != &singles.back())
					stream << ",";
				stream << std::endl;
			}
		}
};

// Entry point
int main(int argc, char *argv[])
{
	// Get arguments
	if (argc < 3)
	{
		std::cout << "usage: MkCht out.h in.json" << std::endl;
		return 0;
	}

	std::string arg_out_h = argv[1];

	// Process charts
	try
	{
		Chart chart(argv[2]);

		std::ofstream out_stream(arg_out_h);
		if (!out_stream.is_open())
			throw RuntimeError(std::string("Failed to open ") + arg_out_h);
		
		chart.Out(out_stream);
	}
	catch (const std::exception &e)
	{
		std::cerr << e.what() << std::endl;
		return 1;
	}

	return 0;
}
