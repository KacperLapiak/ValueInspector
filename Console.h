#ifndef CONSOLE_H
#define CONSOLE_H

#include "Headers.h"

using namespace std;

class Console
{
public:
	string command_;
	string argv_[10];
	int argc_ = 2;

	void consConvMain(int margc, char* margv[])
	{
		for (int i = 1; i < margc; i++)
		{
			for (int a = 0; margv[i][a] != '\0'; a++)
				command_ += margv[i][a];

			if (i < margc - 1) command_ += ' ';
		}

		consArgcArgv();
	}
	void consArgcArgv()
	{
		consResetArgcArgv();
		argv_[0] = command_;

		for (int i = 0; i < command_.length(); i++)
		{
			if (argc_ == size(argv_)+1) break;
			if (command_[i] == ' ') argc_++;
			else argv_[argc_ - 1] += command_[i];
		}
	}
	void consResetArgcArgv()
	{
		for (int i = 0; i < size(argv_); i++)
			argv_[i] = "\0";

		argc_ = 2;
	}
	void consArgcArgvTest()
	{
		cout << dec << "argc_ = " << argc_ << "\n";
		for (int i = 0; i < argc_; i++)
			cout << "argv[" << i << "]_ = " << argv_[i] << "\n";
		cout << "\n";
	}
	void consErrorLevel(const string& errCode){ }
};

#endif