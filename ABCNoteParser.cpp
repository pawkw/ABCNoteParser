// ABCNoteParser.cpp : Defines the entry point for the console application.
//

// #include "stdafx.h" // Required for Visual Studio.
#include <iostream>
#include <string>
#include <sstream>
#include <Windows.h>

using namespace std;

const float NOTE_DURATION = 0.25;
const string INSTRUMENT = "triangle";

bool isNote(char x);
bool isDecoration(char x);
string translateNote(string& ABCNote);
float translateDuration(string& ABCDur);
float parseFraction(string& ABCString, float dur);
void consume(char token, string& target);
float translateTie(string& ABCString, float dur);
void parseChord(string& song, string& note, float& duration);

int main()
{
	// string song = "A3 c/d/e/c/-c d2-|d6 d/e/d/c/|B3 A GE A2-|A8|";
	// string song = "CEG|c3|d3|BA/G-G/|A3/2B/A|Gzz|CDE|G3|A3|G3/2D/D|D3";
	// string song = "CEG[C4E4G4]";
	string song = "|: [c4a4] [B4g4]|efed c2cd|e2f2 gaba|g2e2 e2fg| \
		a4 g4 | efed cdef | g2d2 efed | c2A2 A4 :|";
	stringstream cmd;

	while(song.length() > 0)
	{
		char token = song.at(0);
		string note = "";
		float dur = 0.0;

		if (isNote(token))
		{
			note = translateNote(song);
			dur = translateDuration(song);
			cout << "Note: " << note << " " << (dur * NOTE_DURATION) << "ms." << endl;
		}
		else if (token == 'z' || token == 'Z')
		{
			note = "z";
			consume(token, song);
			dur = translateDuration(song);
			cout << "Rest " << (dur * NOTE_DURATION) << "ms." << endl;
		}
		else if (token == '[')
		{
			parseChord(song, note, dur);
			cout << "Chord: " << note << " " << (dur * NOTE_DURATION) << "ms." << endl;
		}
		else if (isDecoration(token))
		{
			cout << "Decoration: " << token << endl;
			consume(token, song);
			continue;
		}

		if (note == "z") // A rest.
		{
			Sleep(dur*1000);
		}
		else // An actual note or chord.
		{
			cmd << "C:\\\"Program Files (x86)\"\\sox-14-4-1\\play -q -n synth " << (dur * NOTE_DURATION) << " " << note;
			system(cmd.str().c_str());
			cmd.str("");
		}
	}

	// system("pause"); // Visual Studio exits console immediately.
    return 0;
}

// Predicates
bool isNote(char x)
{
	return (x >= 'a' && x <= 'g') || (x >= 'A' && x <= 'G');
}

bool isDecoration(char x)
{
	return (x == ' ' || x == '|' || x == '\t' || x == '\n' || x == ':');
}

// Helper function

void consume(char token, string& target)
{
	if (target.at(0) != token)
	{
		cout << "ERROR: Expected '" << token << "'. Found '" << target.at(0) << "'." << endl << endl;
		system("pause");
		exit(1);
	}

	target = target.substr(1);
}

// Regular functions

string translateNote(string& ABCNote) {
	stringstream noteStream;
	char token = ' ';

	if (ABCNote.length() < 1)
		return "EOS";

	token = ABCNote.at(0);

	if (token >= 'a' && token <= 'g')
		noteStream << (char)(token&95) << "5"; // Convert to uppercase.
	else if (token >= 'A' && token <= 'G')
		noteStream << token << "4";
	else
		noteStream << "Error";

	consume(token, ABCNote);

	return INSTRUMENT+" "+noteStream.str();
}

float translateDuration(string& ABCDur)
{
	float dur = 1.0;
	char token = ' ';

	// If end-of-string, return.
	if (ABCDur.length() < 1)
		return 1.0;

	token = ABCDur.at(0);

	// Check if token is a number or a slash.
	if (token == '/') // A slash immediately after means half length.
	{
		dur = 0.5;
		consume('/', ABCDur);
	}
	else if (token >= '0' && token <= '9')
	{
		dur = token - '0';
		consume(token, ABCDur);
		// now we have to check if the number is followed by a slash.
		dur = parseFraction(ABCDur, dur);
	}

	dur = translateTie(ABCDur, dur);
	return dur;
}


// For cases like 'C3/2'.
float parseFraction(string& ABCString, float dur)
{
	float denominator;
	char token = ' ';

	if (ABCString.length() < 1) // No more string left.
		return dur;

	token = ABCString.at(0);

	if (token != '/')
		return dur; // There is no modifier, the duration is * 1.

	consume('/', ABCString); // Consume the /.
	token = ABCString.at(0); // Get the next token.

	if (token >= '0' && token <= '9')
	{
		denominator = token - '0';
		// Consume the digit.
		consume(token, ABCString);
	}
	else // There is no denominator, assume 2.
		denominator = 2.0;

	return dur / denominator;
}

float translateTie(string& ABCString, float dur)
{
	char token = ' ';

	if (ABCString.length() < 1) // No more string left.
		return dur;

	token = ABCString.at(0);

	if (token != '-')
		return dur; // It is not a tie.

	cout << "Parsed a tie" << endl;

	consume('-', ABCString); // Consume the -.
	token = ABCString.at(0); // Get the next token.
	if (token == '|')
	{
		consume('|', ABCString); // Consume the |.
		token = ABCString.at(0); // Get the next token.
	}
	consume(token, ABCString); // Consume the note. We are going to assume it is the same note."

	return translateDuration(ABCString) + dur;
}

void parseChord(string& song, string& note, float& duration)
{
	consume('[', song);

	char token = song.at(0);

	while (isNote(token))
	{
		note += translateNote(song) + " ";
		duration = translateDuration(song); // The previous duration is discarded. Only the last duration is kept.
		token = song.at(0);
	}
	note += " remix -";
	consume(']', song); // This will stop the program if the chord is malformed.
}
