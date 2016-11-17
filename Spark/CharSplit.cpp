#include "stdafx.h"
#include "CharSplit.h"

StringList::StringList(int Length = 0, int initialmemreserve = 5){
	strings.resize(Length);
	strings.reserve(initialmemreserve);
}

StringList::~StringList(){
	for (int c = strings.size()-1; c >= 0; c--){
		delete strings.at(c);
	}
}


StringList * SplitChar(const char * text, char delimiters[]){
	char * editabletext = new char[strlen(text) + 1]; // For Const Char Strings Passed To This Function
	strcpy(editabletext, text);
	StringList * sl = new StringList();
	char * token = strtok(editabletext, delimiters);
	while (token != nullptr){
		char * curstr = new char[strlen(token)+1];
		strcpy(curstr, token);
		sl->Add(curstr);
		token = strtok(nullptr, delimiters);
	}
	delete editabletext;
	return sl;
}