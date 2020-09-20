#ifndef VALUEINSPECTOR_H
#define VALUEINSPECTOR_H

#include "Headers.h"
#include "Console.h"
#include "Process.h"

using namespace std;

class ValueInspector
{
private:
	vector <char> generalBuffer_;
	string readMode_;
	string whatIsOpen_;
	int valueAddress_;
	int startOffset_;
	int sizeValue_;
	int positiveRows_;
	int negativeRows_;
	int bytesPerRow_;
	int bytesPerCol_;
	int bytesPerPage_;

	// handles: cmd, console, process
	HANDLE hcmd_;
	Console hcon_;
	Process hpro_;

	// template: theme colors
	struct theme
	{
		int area_;
		int head_;
		int offset_;
		int body_;
		int hlBody_;
		int hlAscOne_;
		int hlAscTwo_;
		int hlOneByte_;
		int specCode_;
	};
	theme th_ = {};

	// template: extern file attributes
	struct externFile
	{
		int fileSize_ = 0;
		int pagesNumber_ = 0;
		int currentPage_ = 1;
		int currentOffset_ = 0;
		int lastPageBytes_ = 0;
	};
	externFile ef_ = {};

	// cmd functions
	void setCmdColor(int color)
	{
		SetConsoleTextAttribute(hcmd_, color);
	}
	void setCmdBuffer(int x, int y)
	{
		COORD size;
		size.X = x;
		size.Y = y;
		SetConsoleScreenBufferSize(hcmd_, size);
	}

	// helpful functions
	string setSpecialCode(char byte)
	{
		if (byte == 0) return "NUL";
		else if (byte == 1) return "SOH";
		else if (byte == 2) return "STX";
		else if (byte == 3) return "ETX";
		else if (byte == 4) return "EOT";
		else if (byte == 5) return "ENQ";
		else if (byte == 6) return "ACK";
		else if (byte == 7) return "BEL";
		else if (byte == 8) return "BS";
		else if (byte == 9) return "HT";
		else if (byte == 10) return "LF";
		else if (byte == 11) return "VT";
		else if (byte == 12) return "FF";
		else if (byte == 13) return "CR";
		else if (byte == 14) return "SO";
		else if (byte == 15) return "SI";
		else if (byte == 16) return "DLE";
		else if (byte == 17) return "DC1";
		else if (byte == 18) return "DC2";
		else if (byte == 19) return "DC3";
		else if (byte == 20) return "DC4";
		else if (byte == 21) return "NAK";
		else if (byte == 22) return "SYN";
		else if (byte == 23) return "ETB";
		else if (byte == 24) return "CAN";
		else if (byte == 25) return "EM";
		else if (byte == 26) return "SUB";
		else if (byte == 27) return "ESC";
		else if (byte == 28) return "FS";
		else if (byte == 29) return "GS";
		else if (byte == 30) return "RS";
		else if (byte == 31) return "US";
		else if (byte == 127) return "DEL";
	}
	string retCurrentArea()
	{
		if (whatIsOpen_ == "file") return to_string(ef_.currentPage_) + "/" + to_string(ef_.pagesNumber_);
		else if (whatIsOpen_ == "eproc") return "pid " + to_string(hpro_.currentPid_);
		else if (whatIsOpen_ == "tproc" && valueAddress_ < 0x500000) return "stack";
		else if (whatIsOpen_ == "tproc" && valueAddress_ > 0x500000) return "heap";
	}
	void setBytesPerPage()
	{
		bytesPerPage_ = bytesPerRow_ * (positiveRows_ + negativeRows_);
	}
	void setFileSize(fstream& hfile)
	{
		long positionBuffer = hfile.tellg();
		hfile.seekp(0, ios_base::end);
		ef_.fileSize_ = hfile.tellg();
		hfile.seekp(positionBuffer, ios_base::beg);
	}
	void setPagesNumber(fstream& hfile)
	{
		long positionBuffer = hfile.tellg();
		hfile.seekp(0, ios_base::end);
		ef_.pagesNumber_ = ceil((float)hfile.tellg() / bytesPerPage_);
		hfile.seekp(positionBuffer, ios_base::beg);
	}
	void resetFileAttributes()
	{
		ef_.fileSize_ = 0;
		ef_.pagesNumber_ = 0;
		ef_.currentPage_ = 1;
		ef_.currentOffset_ = 0;
		ef_.lastPageBytes_ = 0;
	}
	void showAvalibleColors()
	{
		for (int i = 0; i < 256; i++)
		{
			if (i % 10 == 0) cout << "\n\t";
			setCmdColor(i);
			cout.fill('0');
			cout.width(3);
			cout << dec << i;
			setCmdColor(1);
			cout << " ";
		}

		setCmdColor(10);
		cout << "\n\n\tpress [ENTER] for exit\n\t";
		cin.get();
	}
	bool loadUserConfig()
	{
		fstream hfile("config.txt", ios_base::in);
		if (hfile.is_open())
		{
			string tempArgv[4]{ "inner", "imp", "cfg", "config.txt" };
			commandImp(size(tempArgv), tempArgv);
			hfile.close();
			return 1;
		}
		else
		{
			hcon_.consErrorLevel("errLoadUserConfig");
			return 0;
		}
	}

	// command definitions
	void commandHelp(int argc, string* argv)
	{
		if (argc == 2 && argv[1] == "help")
		{
			cout << "\n";
			cout << "\t   For more information about specific command          \n";
			cout << "\t   type help command with the selected other one.       \n";
			cout << "\t   Example: help bg                                   \n\n";
			cout << "\t+------------------------------------------------------+\n";
			cout << "\t|  command [argument]       |  example arguments       |\n";
			cout << "\t|------------------------------------------------------|\n";
			cout << "\t|  jmp [offset]             |  0x30fafa, proc, -10, up |\n";
			cout << "\t|  bg  [background_name]    |  skysun, bloody          |\n";
			cout << "\t|  bf  [byte_format]        |  hex, dec, oct, asc      |\n";
			cout << "\t|  bt  [byte_struct]        |  bpr, bpc, pr, nr...     |\n";
			cout << "\t|  exp [look_example]       |  data, cfg...            |\n";
			cout << "\t|  imp [source_path]        |  data, cfg...            |\n";
			cout << "\t|  procs                    |  [no arguments]          |\n";
			cout << "\t+------------------------------------------------------+\n";
			cout << "\n\t   press [ENTER] for exit\n\t";
		}
		else if (argc == 3 && argv[2] == "jmp")
		{
			cout << "\n";
			cout << "\t   This command let you jump to indicated address,      \n";
			cout << "\t   jump a indicated number of bytes forward and back,   \n";
			cout << "\t   or jump one row up and down.                       \n\n";
			cout << "\t+------------------------------------------------------+\n";
			cout << "\t|  command               |  explanation                |\n";
			cout << "\t+------------------------------------------------------+\n";
			cout << "\t|  jmp 0x003ffaab        |  jump to indicated address  |\n";
			cout << "\t|  jmp 10                |  jump 10 bytes forward      |\n";
			cout << "\t|  jmp -10               |  jump 10 bytes back         |\n";
			cout << "\t|  jmp up                |  jump one row up            |\n";
			cout << "\t|  jmp down              |  jump one row down          |\n";
			cout << "\t+------------------------------------------------------+\n";
			cout << "\n";
			cout << "\t+------------------------------------------------------+\n";
			cout << "\t|  jmp proc [PID]                                      |\n";
			cout << "\t|  example: jmp proc 1203                              |\n";
			cout << "\t+------------------------------------------------------+\n";
			cout << "\t|  This command let you jump to the first avalible     |\n";
			cout << "\t|  address of the indicated process. You can check     |\n";
			cout << "\t|  out PID of specified process using command procs.   |\n";
			cout << "\t+------------------------------------------------------+\n";
			cout << "\n";
			cout << "\t+------------------------------------------------------+\n";
			cout << "\t|  jmp proc [PID] [offset]                             |\n";
			cout << "\t|  example: jmp proc 1203 0x000A5000                   |\n";
			cout << "\t+------------------------------------------------------+\n";
			cout << "\t|  This command let you jump to the indicated address  |\n";
			cout << "\t|  of the indicated process. If the address isn't      |\n";
			cout << "\t|  avalible to read, then jump to the first address    |\n";
			cout << "\t|  that can be read.                                   |\n";
			cout << "\t+------------------------------------------------------+\n";
			cout << "\n\t   press [ENTER] for exit\n\t";
		}
		else if (argc == 3 && argv[2] == "bg")
		{
			cout << endl;
			cout << "\t   This command let you change color theme.             \n";
			cout << "\t   The [number] has to be between 0 and 255.          \n\n";
			cout << "\t+------------------------------------------------------+\n";
			cout << "\t|  command              |  explanation                 |\n";
			cout << "\t+------------------------------------------------------+\n";
			cout << "\t|  bg colors            |  show avalible colors        |\n";
			cout << "\t|  bg [theme]           |  choose color theme          |\n";
			cout << "\t|  bg area [number]     |  change color of the area    |\n";
			cout << "\t|  bg head [number]     |  --//--                      |\n";
			cout << "\t|  bg offset [number]   |  --//--                      |\n";
			cout << "\t|  bg body [number]     |  --//--                      |\n";
			cout << "\t|  bg hlbody [number]   |  highlighted data color      |\n";
			cout << "\t|  bg hlone [number]    |  --//-- ascii mode color     |\n";
			cout << "\t|  bg hltwo [number]    |  --//-- ascii mode color     |\n";
			cout << "\t|  bg hloneb [number]   |  one byte highlighted color  |\n";
			cout << "\t|  bg spec [number]     |  special sign color          |\n";
			cout << "\t+------------------------------------------------------+\n";
			cout << "\t   themes: classic, bloody, skysun, gynvael, blogbyte";
			cout << "\n\t   press [ENTER] for exit\n\t";
		}
		else if (argc == 3 && argv[2] == "bf")
		{
			cout << "\n";
			cout << "\t   This command let you choose byte format. \n\n";
			cout << "\t+------------------------------------------------------+\n";
			cout << "\t|  command              |  explanation                 |\n";
			cout << "\t+------------------------------------------------------+\n";
			cout << "\t|  bf hex               |  hexadecimal                 |\n";
			cout << "\t|  bf oct               |  octal                       |\n";
			cout << "\t|  bf dec               |  decimal                     |\n";
			cout << "\t|  bf asc               |  ascii                       |\n";
			cout << "\t+------------------------------------------------------+\n";
			cout << "\n\t   press [ENTER] for exit\n\t";
		}
		else if (argc == 3 && argv[2] == "bt")
		{
			cout << "\n";
			cout << "\t   This command let you change bytes arragement.        \n";
			cout << "\t+------------------------------------------------------+\n";
			cout << "\t|  command                   |  explanation            |\n";
			cout << "\t|------------------------------------------------------|\n";
			cout << "\t|  bt bpr 8                  |  set 16 bytes per row   |\n";
			cout << "\t|  bt bpc 2                  |  set 2 bytes per column |\n";
			cout << "\t|  bt pr 15                  |  set 5 positive rows    |\n";
			cout << "\t|  bt nr 5                   |  set 5 negative rows    |\n";
			cout << "\t+------------------------------------------------------+\n";
			cout << "\n\t   press [ENTER] for exit\n\t";
		}
		else if (argc == 3 && argv[2] == "exp")
		{
			cout << "\n";
			cout << "\t   This command let you export color theme              \n";
			cout << "\t   configuration and export actual bytes wiev in        \n";
			cout << "\t   specific format.                                     \n";
			cout << "\t+------------------------------------------------------+\n";
			cout << "\t|  command                   |  explanation            |\n";
			cout << "\t|------------------------------------------------------|\n";
			cout << "\t|  exp cfg [path]            |  export color theme cfg |\n";
			cout << "\t+------------------------------------------------------+\n";
			cout << "\n\t   press [ENTER] for exit\n\t";
		}
		else if (argc == 3 && argv[2] == "imp")
		{
			cout << "\n";
			cout << "\t   This command let you import color theme              \n";
			cout << "\t   configuration and browse extern file (bin, txt).     \n";
			cout << "\t+------------------------------------------------------+\n";
			cout << "\t|  command                   |  explanation            |\n";
			cout << "\t|------------------------------------------------------|\n";
			cout << "\t|  imp cfg [path]            |  import color theme cfg |\n";
			cout << "\t|  imp data [path]           |  browse extern file     |\n";
			cout << "\t+------------------------------------------------------+\n";
			cout << "\n\t   press [ENTER] for exit\n\t";
		}
		else if (argc == 3 && argv[2] == "procs")
		{
			cout << "\n\t   This command shows list of running processes.      \n";
			cout << "\n\t   press [ENTER] for exit                           \n\t";
		}
	}
	void commandJmp(int argc, string* argv)
	{
		sizeValue_ = 1;
		th_.hlAscOne_ = th_.hlOneByte_;

		if (argc == 3 && argv[2] == "up")
		{
			valueAddress_ += bytesPerRow_;
		}
		else if (argc == 3 && argv[2] == "down")
		{
			valueAddress_ -= bytesPerRow_;
		}
		else if (argc == 3 && argv[2][1] == 'x')
		{
			string strBuf;
			for (int i = 2; i < argv[2].length(); i++)
				strBuf += argv[2][i];

			// to trzeba lepiej zaimplementowaæ
			if (hpro_.isAddrOpen(GetCurrentProcessId(), stoi(strBuf, 0, 16)))
			{
				valueAddress_ = stoi(strBuf, 0, 16);
			}
			else
			{
				hcon_.consErrorLevel("errAddrRead");
			}

		}
		else if (isdigit(argv[2][0]) || (argv[2][0] == '-' && isdigit(argv[2][1])))
		{
			string strBuf;
			if (isdigit(argv[2][0]))
			{
				for (int i = 0; i < argv[2].length(); i++)
					strBuf += argv[2][i];

				valueAddress_ += stoi(strBuf);
			}
			else if (argv[2][0] == '-')
			{
				for (int i = 1; i < argv[2].length(); i++)
					strBuf += argv[2][i];

				valueAddress_ -= stoi(strBuf);
			}
		}
		else if (argc > 3 && argv[2] == "proc")
		{
			whatIsOpen_ = "eproc";
			readMode_ = "asc";

			string strBuf;
			if (argc == 5 && argv[4][1] == 'x')
			{
				for (int i = 2; i < argv[4].length(); i++)
					strBuf += argv[4][i];

				hpro_.startAddress_ = stoi(strBuf, 0, 16);
			}

			hpro_.readProcMem(stoi(argv[3]), bytesPerPage_);

			generalBuffer_.clear();
			for (int i = 0; i < hpro_.bytesBuffer_.size(); i++)
				generalBuffer_.push_back(hpro_.bytesBuffer_[i]);

			bytesPerPage_ = generalBuffer_.size();
			startOffset_ = hpro_.startAddress_ - hpro_.readBytes_;
			refreshScreen();
			cout << "\n\n\t(<)previous, (>)next, (q)exit, (c)console";

			int relativePage = 0;
			while (true)
			{
				char key = _getch();
				if (key == '.')
				{
					startOffset_ += hpro_.readBytes_;
					relativePage++;
				}
				else if (key == ',')
				{
					if (relativePage == 0) continue;
					relativePage--;
					startOffset_ -= hpro_.readBytes_;
					hpro_.startAddress_ -= 2 * hpro_.bytesBuffer_.size();
				}
				else if (key == 'q')
				{
					whatIsOpen_ = "tproc";
					break;
				}
				else if (key == 'c')
				{
					refreshScreen();
					showConsole(NULL, NULL);
				}
				generalBuffer_.clear();
				hpro_.readProcMem(stoi(argv[3]), bytesPerPage_);
				for (int i = 0; i < hpro_.bytesBuffer_.size(); i++)
					generalBuffer_.push_back(hpro_.bytesBuffer_[i]);
				bytesPerPage_ = generalBuffer_.size();

				refreshScreen();
				cout << "\n\n\t(<)previous, (>)next, (q)exit, (c)console";
			}
		}
	}
	void commandBg(int argc, string* argv)
	{
		if (argv[0] == "bg classic") setColorTheme(argv[2]);
		else if (argv[0] == "bg bloody") setColorTheme(argv[2]);
		else if (argv[0] == "bg skysun") setColorTheme(argv[2]);
		else if (argv[0] == "bg gynvael") setColorTheme(argv[2]);
		else if (argv[0] == "bg blogbyte") setColorTheme(argv[2]);
		else if (argv[1] == "bg" && argv[2] == "area") th_.area_ = stoi(argv[3]);
		else if (argv[1] == "bg" && argv[2] == "head") th_.head_ = stoi(argv[3]);
		else if (argv[1] == "bg" && argv[2] == "body") th_.body_ = stoi(argv[3]);
		else if (argv[1] == "bg" && argv[2] == "offset") th_.offset_ = stoi(argv[3]);
		else if (argv[1] == "bg" && argv[2] == "hlbody") th_.hlBody_ = stoi(argv[3]);
		else if (argv[1] == "bg" && argv[2] == "hlone") th_.hlAscOne_ = stoi(argv[3]);
		else if (argv[1] == "bg" && argv[2] == "hltwo") th_.hlAscTwo_ = stoi(argv[3]);
		else if (argv[1] == "bg" && argv[2] == "hloneb") th_.hlOneByte_ = stoi(argv[3]);
		else if (argv[1] == "bg" && argv[2] == "spec") th_.specCode_ = stoi(argv[3]);
		else if (argv[1] == "bg" && argv[2] == "colors") showAvalibleColors();
	}
	void commandBf(int argc, string* argv)
	{
		if (argv[0] == "bf hex") readMode_ = argv[2];
		else if (argv[0] == "bf dec") readMode_ = argv[2];
		else if (argv[0] == "bf oct") readMode_ = argv[2];
		else if (argv[0] == "bf asc") readMode_ = argv[2];
	}
	void commandBt(int argc, string* argv)
	{
		if (argv[2] == "bpr") bytesPerRow_ = stoi(argv[3]);
		else if (argv[2] == "bpc") bytesPerCol_ = stoi(argv[3]);
		else if (argv[2] == "pr") positiveRows_ = stoi(argv[3]);
		else if (argv[2] == "nr") negativeRows_ = stoi(argv[3]);
		setBytesPerPage();
	}
	void commandExp(int argc, string* argv)
	{
		if (argv[2] == "cfg")
		{
			if (argv[3] == "") argv[3] = "theme.txt";
			fstream hfile(argv[3], ios_base::out, ios_base::trunc);
			if (hfile.is_open())
			{
				hfile << noskipws << "pRows.............= " << positiveRows_ << "\n";
				hfile << noskipws << "nRows.............= " << negativeRows_ << "\n";
				hfile << noskipws << "bytesPerRow.......= " << bytesPerRow_ << "\n";
				hfile << noskipws << "bytesPerCol.......= " << bytesPerCol_ << "\n";
				hfile << noskipws << "areaColor.........= " << th_.area_ << "\n";
				hfile << noskipws << "headColor.........= " << th_.head_ << "\n";
				hfile << noskipws << "offsetColor.......= " << th_.offset_ << "\n";
				hfile << noskipws << "bodyColor.........= " << th_.body_ << "\n";
				hfile << noskipws << "hlBodyColor.......= " << th_.hlBody_ << "\n";
				hfile << noskipws << "hlColorAscOne.....= " << th_.hlAscOne_ << "\n";
				hfile << noskipws << "hlColorAscTwo.....= " << th_.hlAscTwo_ << "\n";
				hfile << noskipws << "hlColorOneByte....= " << th_.hlOneByte_ << "\n";
				hfile << noskipws << "specSignColor.....= " << th_.specCode_ << "\n";
				hfile.close();

				hcon_.consErrorLevel("okConfigExport");
			}
			else
			{
				hcon_.consErrorLevel("errFileOpen");
			}
		}
		else if (argc == 4 && argv[2] == "dump")
		{
			fstream hfile(argv[3], ios_base::in | ios_base::out | ios_base::trunc);
			if (hfile.is_open())
			{
				for (int i = 0; i < generalBuffer_.size(); i++)
					hfile << generalBuffer_[i];
			}
			else
			{
				hcon_.consErrorLevel("errFileOpen");
			}
		}
	}
	void commandImp(int argc, string* argv)
	{
		if (argv[2] == "data")
		{
			resetFileAttributes();

			fstream hfile(argv[3], ios_base::in | ios_base::binary);
			if (hfile.is_open())
			{
				generalBuffer_.clear();
				whatIsOpen_ = "file";
				startOffset_ = 0;
				sizeValue_ = 0;
				setPagesNumber(hfile);

				for (int i = 0; i < bytesPerPage_; i++)
				{
					char ch;
					hfile >> noskipws >> ch;
					generalBuffer_.push_back(ch);
				}

				refreshScreen();
				cout << "\n\n\t(<)previous, (>)next, (q)exit, (c)console";

				while (true)
				{
					char key = _getch();
					if (key == '.' && ef_.currentPage_ < ef_.pagesNumber_)
					{
						ef_.currentPage_++;
						generalBuffer_.clear();
						for (int i = 0; i < bytesPerPage_; i++)
						{
							char ch;
							if (!(hfile >> noskipws >> ch))
							{
								ef_.lastPageBytes_ = i;
								bytesPerPage_ = i;
								hfile.clear();
								break;
							}

							generalBuffer_.push_back(ch);
							startOffset_++;
						}

						refreshScreen();
						cout << "\n\n\t(<)previous, (>)next, (q)exit, (c)console";
					}
					else if (key == ',' && ef_.currentPage_ > 1)
					{
						generalBuffer_.clear();

						if (ef_.currentPage_ == ef_.pagesNumber_)
						{
							setBytesPerPage();
							hfile.seekp(-(bytesPerPage_ + ef_.lastPageBytes_), ios_base::cur);
							startOffset_ -= ef_.lastPageBytes_;
						}
						else
						{
							hfile.seekp(-bytesPerPage_ * 2LL, ios_base::cur);
							startOffset_ -= bytesPerPage_;
						}

						for (int i = 0; i < bytesPerPage_; i++)
						{
							char ch;
							hfile >> noskipws >> ch;
							generalBuffer_.push_back(ch);
						}

						ef_.currentPage_--;
						refreshScreen();
						cout << "\n\n\t(<)previous, (>)next, (q)exit, (c)console";
					}
					else if (key == 'q')
					{
						hfile.close();
						break;
					}
					else if (key == 'c')
					{
						refreshScreen();
						showConsole(NULL, NULL);
					}
				}
			}
		}
		else if (argv[2] == "cfg")
		{
			fstream handle(argv[3], ios_base::in);
			if (argv[3] == "") argv[3] = "config.txt";
			if (handle.is_open())
			{
				string buf;
				int th_tab[13]{};

				int cntr = 0;
				while (getline(handle, buf))
				{
					th_tab[cntr] = stoi(buf.substr(20, 24));
					cntr++;
				}

				positiveRows_ = th_tab[0];
				negativeRows_ = th_tab[1];
				bytesPerRow_ = th_tab[2];
				bytesPerCol_ = th_tab[3];
				th_.area_ = th_tab[4];
				th_.head_ = th_tab[5];
				th_.offset_ = th_tab[6];
				th_.body_ = th_tab[7];
				th_.hlBody_ = th_tab[8];
				th_.hlAscOne_ = th_tab[9];
				th_.hlAscTwo_ = th_tab[10];
				th_.hlOneByte_ = th_tab[11];
				th_.specCode_ = th_tab[12];

				handle.close();

				if (!(argv[0] == "inner")) hcon_.consErrorLevel("okConfigImport");
			}
			else hcon_.consErrorLevel("errFileOpen");
		}
	}
	void commandProcs(int argc, string* argv)
	{
		if (argv[1] == "procs") readMode_ = "pro";
	}

	// display formats
	void formatDigit(int colWidth)
	{
		if (whatIsOpen_ == "tproc")
		{
			// area format
			cout.fill(' ');
			cout.width(11);
			SetConsoleTextAttribute(hcmd_, th_.area_);
			cout << left << " " + retCurrentArea();

			// head format
			int shift = (valueAddress_ % 16);
			int sep = 0;
			for (int i = shift; i < bytesPerRow_ + shift; i++)
			{
				SetConsoleTextAttribute(hcmd_, th_.head_);
				if (sep % bytesPerCol_ == 0 && i != (-bytesPerRow_ * negativeRows_)) cout << " ";
				cout.width(colWidth + 1LL);
				cout << uppercase << left << hex << i;
				sep++;
			}

			cout << " \n";

			cout.fill('0');
			for (int i = -(bytesPerRow_ * negativeRows_); i < bytesPerRow_ * positiveRows_; i++)
			{
				// line break
				if (i % bytesPerCol_ == 0 && i != (-bytesPerRow_ * negativeRows_)) cout << " ";
				if (i % bytesPerRow_ == 0 && i != (-bytesPerRow_ * negativeRows_)) cout << endl;

				// offset format
				if (i % bytesPerRow_ == 0)
				{
					SetConsoleTextAttribute(hcmd_, th_.offset_);
					cout << " ";
					cout.width(8);
					cout << nouppercase << right << hex << int(&((unsigned char*)valueAddress_)[i]) << " |";

					SetConsoleTextAttribute(hcmd_, th_.body_);
					cout << " ";
				}

				// body format
				SetConsoleTextAttribute(hcmd_, th_.body_);
				if (i >= 0) SetConsoleTextAttribute(hcmd_, th_.hlBody_);
				if (sizeValue_ < i + 1) SetConsoleTextAttribute(hcmd_, th_.body_);

				cout.width(colWidth);

				if (readMode_ == "hex")	cout << uppercase << hex << int(((unsigned char*)valueAddress_)[i]) << " ";
				else if (readMode_ == "dec") cout << dec << int(((unsigned char*)valueAddress_)[i]) << " ";
				else if (readMode_ == "oct") cout << oct << int(((unsigned char*)valueAddress_)[i]) << " ";
			}

			cout << " ";
			SetConsoleTextAttribute(hcmd_, 10);
		}
		else if (whatIsOpen_ == "file" || whatIsOpen_ == "eproc")
		{
			// area format
			cout.fill(' ');
			cout.width(11);
			SetConsoleTextAttribute(hcmd_, th_.area_);
			cout << left << " " + retCurrentArea();

			// head format
			int sep = 0;
			for (int i = 0; i < bytesPerRow_; i++)
			{
				SetConsoleTextAttribute(hcmd_, th_.head_);
				if (sep % bytesPerCol_ == 0 && i != (-bytesPerRow_ * negativeRows_)) cout << " ";
				cout.width(colWidth + 1LL);
				cout << uppercase << left << hex << i;
				sep++;
			}

			cout.fill('0');
			ef_.currentOffset_ = startOffset_;
			for (int i = 0; i < bytesPerPage_; i++)
			{
				// line break
				if (i % bytesPerCol_ == 0 && i != (-bytesPerRow_ * negativeRows_)) cout << " ";
				if (i % bytesPerRow_ == 0 && i != (-bytesPerRow_ * negativeRows_)) cout << endl;

				// offset format
				if (i % bytesPerRow_ == 0)
				{
					setCmdColor(th_.offset_);
					cout << " ";
					cout.width(8);
					cout << nouppercase << right << hex << ef_.currentOffset_ << " |";

					ef_.currentOffset_ += bytesPerRow_;

					setCmdColor(th_.body_);
					cout << " ";
				}

				// body format
				SetConsoleTextAttribute(hcmd_, th_.body_);
				if (i >= 0) SetConsoleTextAttribute(hcmd_, th_.hlBody_);
				if (sizeValue_ < i + 1) SetConsoleTextAttribute(hcmd_, th_.body_);

				cout.width(colWidth);

				if (readMode_ == "hex")	cout << uppercase << hex << (int)(unsigned char)generalBuffer_[i] << " ";
				else if (readMode_ == "dec") cout << dec << (int)(unsigned char)generalBuffer_[i] << " ";
				else if (readMode_ == "oct") cout << oct << (int)(unsigned char)generalBuffer_[i] << " ";
			}
			cout << " ";

			setCmdColor(10);
		}
	}
	void formatAscii()
	{
		if (whatIsOpen_ == "eproc" || whatIsOpen_ == "file")
		{
			// area format
			cout.fill(' ');
			cout.width(11);
			setCmdColor(th_.area_);
			cout << left << " " + retCurrentArea();

			// head format
			for (int i = 0; i < bytesPerRow_; i++)
			{
				setCmdColor(th_.head_);
				cout.width(3);
				cout << uppercase << right << hex << i;
			}

			cout << " \n";

			cout.fill('0');
			ef_.currentOffset_ = startOffset_;
			for (int i = 0; i < generalBuffer_.size(); i++)
			{
				// line break
				if (i != 0 && i % bytesPerRow_ == 0) cout << endl;

				// offset format
				if (i % bytesPerRow_ == 0)
				{
					setCmdColor(th_.offset_);
					cout << " ";
					cout.width(8);
					cout << nouppercase << hex << ef_.currentOffset_ << " |";

					ef_.currentOffset_ += bytesPerRow_;

					setCmdColor(th_.body_);
					cout << " ";
				}

				// body format
				setCmdColor(th_.body_);
				cout.fill(' ');
				cout.width(3);

				char byte = generalBuffer_[i];
				if (byte > 31 || byte < 0)
				{
					cout.width(2);
					cout << byte << " ";
				}
				else
				{
					setCmdColor(th_.specCode_);
					cout << setSpecialCode(byte);
				}
				cout.fill('0');
			}
			setCmdColor(10);
		}
		else if (whatIsOpen_ == "tproc")
		{
			// area format
			cout.fill(' ');
			cout.width(11);
			setCmdColor(th_.area_);
			cout << left << " " + retCurrentArea();

			// head format
			int shift = (valueAddress_ % 16);
			for (int i = shift; i < bytesPerRow_ + shift; i++)
			{
				setCmdColor(th_.head_);
				cout.width(3);
				cout << uppercase << right << hex << i;
			}

			cout << " \n";

			cout.fill('0');
			for (int i = -bytesPerRow_ * negativeRows_; i < bytesPerRow_ * positiveRows_; i++)
			{
				// line break
				if (i % bytesPerRow_ == 0 && i != (-bytesPerRow_ * negativeRows_)) cout << endl;

				// offset format
				if (i % bytesPerRow_ == 0)
				{
					setCmdColor(th_.offset_);
					cout << " ";
					cout.width(8);
					cout << nouppercase << hex << int(&((unsigned char*)valueAddress_)[i]) << " |";

					setCmdColor(th_.body_);
					cout << " ";
				}

				// body format
				if (i + 1 > sizeValue_ || i < 0) setCmdColor(th_.body_);
				else if (sizeValue_ == 1) setCmdColor(th_.hlOneByte_);
				else if (i % 2 == 0) setCmdColor(th_.hlAscOne_);
				else setCmdColor(th_.hlAscTwo_);

				cout.fill(' ');
				cout.width(3);

				char byte = int(((unsigned char*)valueAddress_)[i]);
				if (byte > 31 || byte < 0)
				{
					cout.width(2);
					cout << byte << " ";
				}
				else if (byte == 0 && i < sizeValue_ && i > 0)
				{
					if (i % 2 == 0) setCmdColor(th_.hlAscOne_);
					else setCmdColor(th_.hlAscTwo_);
					cout << "NUL";
				}
				else
				{
					setCmdColor(th_.specCode_);
					if (sizeValue_ == 1 && i == 0) setCmdColor(th_.hlOneByte_);
					cout << setSpecialCode(byte);
				}
				cout.fill('0');
			}
			setCmdColor(10);
		}
	}
	void formatProcess()
	{
		// PID format
		SetConsoleTextAttribute(hcmd_, th_.area_);
		cout.fill(' ');
		cout.width(8);
		cout << left << dec << " PID";

		// head format
		SetConsoleTextAttribute(hcmd_, th_.head_);
		cout.width(41);
		cout << " process name";

		for (int i = 0; i < hpro_.procsNumber_; i++)
		{
			SetConsoleTextAttribute(hcmd_, th_.offset_);
			cout << endl << " ";
			cout.width(6);
			cout << hpro_.procsPid_[i] << "|";

			SetConsoleTextAttribute(hcmd_, th_.body_);
			cout << " " << hpro_.procsName_[i];
		}
		SetConsoleTextAttribute(hcmd_, 10);
	}

public:
	template <typename T>
	ValueInspector(T& addrValue, int sizeValue)
		: valueAddress_((int)&addrValue), sizeValue_(sizeValue)
	{
		hcmd_ = GetStdHandle(STD_OUTPUT_HANDLE);
		whatIsOpen_ = "tproc";
		startOffset_ = (int)&addrValue;
		readMode_ = "hex";
		positiveRows_ = 15;
		negativeRows_ = 5;
		bytesPerRow_ = 16;
		bytesPerCol_ = 4;
		setBytesPerPage();
		if (!loadUserConfig()) setColorTheme("bloody");
	}
	~ValueInspector() { }

	void setCmdFormat(int x, int y, int px = 0, int py = 0)
	{
		HWND hcmd = GetConsoleWindow();
		MoveWindow(hcmd, px, py, x, y, TRUE);
	}
	void setColorTheme(const string& theme)
	{
		if (theme == "classic")
		{
			th_.area_ = 240;
			th_.head_ = 240;
			th_.offset_ = 15;
			th_.body_ = 15;
			th_.hlBody_ = 240;
			th_.hlAscOne_ = 240;
			th_.hlAscTwo_ = 240;
			th_.hlOneByte_ = 240;
			th_.specCode_ = 15;
		}
		else if (theme == "bloody")
		{
			th_.area_ = 143;
			th_.head_ = 116;
			th_.offset_ = 78;
			th_.body_ = 72;
			th_.hlBody_ = 79;
			th_.hlAscOne_ = 116;
			th_.hlAscTwo_ = 132;
			th_.hlOneByte_ = 112;
			th_.specCode_ = 76;
		}
		else if (theme == "skysun")
		{
			th_.area_ = 143;
			th_.head_ = 112;
			th_.offset_ = 224;
			th_.body_ = 159;
			th_.hlBody_ = 249;
			th_.hlAscOne_ = 224;
			th_.hlAscTwo_ = 224;
			th_.hlOneByte_ = 249;
			th_.specCode_ = 151;
		}
		else if (theme == "gynvael")
		{
			th_.area_ = 143;
			th_.head_ = 117;
			th_.offset_ = 93;
			th_.body_ = 87;
			th_.hlBody_ = 95;
			th_.hlAscOne_ = 245;
			th_.hlAscTwo_ = 117;
			th_.hlOneByte_ = 245;
			th_.specCode_ = 88;
		}
		else if (theme == "blogbyte")
		{
			th_.area_ = 162;
			th_.head_ = 42;
			th_.offset_ = 42;
			th_.body_ = 162;
			th_.hlBody_ = 175;
			th_.hlAscOne_ = 63;
			th_.hlAscTwo_ = 63;
			th_.hlOneByte_ = 63;
			th_.specCode_ = 163;
		}
	}
	void setDispDataFormat(int bytesPerRow, int bytesPerCol, int pRows, int nRows, const string& mode) // complete
	{
		bytesPerRow_ = bytesPerRow;
		bytesPerCol_ = bytesPerCol;
		positiveRows_ = pRows;
		negativeRows_ = nRows;
		readMode_ = mode;
		setBytesPerPage();
	}
	void refreshScreen()
	{
		system("cls");

		if (readMode_ == "hex") formatDigit(2);
		else if (readMode_ == "dec") formatDigit(3);
		else if (readMode_ == "oct") formatDigit(3);
		else if (readMode_ == "asc") formatAscii();
		else if (readMode_ == "pro") formatProcess();
	}
	void showConsole(int margc, char* margv[])
	{
		if (margc > 1)
		{
			hcon_.consConvMain(margc, margv);
			if (hcon_.argv_[1] == "help") commandHelp(hcon_.argc_, hcon_.argv_);
			else if (hcon_.argv_[1] == "jmp") commandJmp(hcon_.argc_, hcon_.argv_);
			else if (hcon_.argv_[1] == "exp") commandExp(hcon_.argc_, hcon_.argv_);
			else if (hcon_.argv_[1] == "imp") commandImp(hcon_.argc_, hcon_.argv_);
			else if (hcon_.argv_[1] == "bg") commandBg(hcon_.argc_, hcon_.argv_);
			else if (hcon_.argv_[1] == "bf") commandBf(hcon_.argc_, hcon_.argv_);
			else if (hcon_.argv_[1] == "bt") commandBt(hcon_.argc_, hcon_.argv_);
			else if (hcon_.argv_[1] == "procs") commandProcs(hcon_.argc_, hcon_.argv_);

			margc = 0;
			refreshScreen();
		}
		else
		{
			cout << "\n\n  >: ";
			while (getline(cin, hcon_.command_))
			{
				hcon_.consArgcArgv();
				if (hcon_.argv_[1] == "help") { commandHelp(hcon_.argc_, hcon_.argv_); continue; }
				else if (hcon_.argv_[1] == "jmp") commandJmp(hcon_.argc_, hcon_.argv_);
				else if (hcon_.argv_[1] == "exp") commandExp(hcon_.argc_, hcon_.argv_);
				else if (hcon_.argv_[1] == "imp") commandImp(hcon_.argc_, hcon_.argv_);
				else if (hcon_.argv_[1] == "bg") commandBg(hcon_.argc_, hcon_.argv_);
				else if (hcon_.argv_[1] == "bf") commandBf(hcon_.argc_, hcon_.argv_);
				else if (hcon_.argv_[1] == "bt") commandBt(hcon_.argc_, hcon_.argv_);
				else if (hcon_.argv_[1] == "procs") commandProcs(hcon_.argc_, hcon_.argv_);

				refreshScreen();
				cout << "\n\n  >: ";
			}
		}
	}
};

#endif