//****************************************************************************************
//
//	INCLUDE FILES
//
//****************************************************************************************
#include	<fstream>
#include	<iostream>
#include	<string>
#include    <algorithm>
//#include	<unistd.h>
#include	<sstream>
#include    <vector>
#include	"DocumentIndex.h"
#include	"GetLine.h"

using namespace std;

//****************************************************************************************
//
//	CONSTANT DEFINITIONS
//
//****************************************************************************************

//****************************************************************************************
//
//	CLASSES, TYPEDEFS AND STRUCTURES
//
//****************************************************************************************
typedef	string::size_type	StringSize;

bool punctCheck(char c)
{
	if (c == ';' || c == ':' || c == ',' || c == '.' || c == '!' || c == '?' || c == '(' || c == ')' || c == '"' || c == '\'' || c == '*')
		return(true);
	else
		return(false);
};

//****************************************************************************************
//
//	PUBLIC DATA
//
//****************************************************************************************

//****************************************************************************************
//
//	PRIVATE DATA
//
//****************************************************************************************

//****************************************************************************************
//
//	FUNCTION PROTOTYPES
//
//****************************************************************************************

//****************************************************************************************
//
//	DocumentFile::Close
//
//****************************************************************************************
void	DocumentFile::Close()
{
	//************************************************************************************
	//	LOCAL DATA

	//************************************************************************************
	//	EXECUTABLE STATEMENTS
	file_.close();
	file_.clear();

	return;
}

//****************************************************************************************
//
//	DocumentFile::GetPageNumber
//
//****************************************************************************************
int	DocumentFile::GetPageNumber()
{
	//************************************************************************************
	//	LOCAL DATA

	//************************************************************************************
	//	EXECUTABLE STATEMENTS

	return(pageNumber_);
}

//****************************************************************************************
//
//	DocumentFile::GetWord
//
//****************************************************************************************
string	DocumentFile::GetWord()
{
	//************************************************************************************
	//	LOCAL DATA
	string	word;
	bool flag = false;

	//************************************************************************************
	//	EXECUTABLE STATEMENTS
	if (words.empty())
	{
		return "";
	}

	while (true)
	{
		it = words.begin();
		word = *it;

		if (hyphenCheck(word))
		{
			if ((++it) != words.end())
			{
				words.erase(words.begin());
				continue;
			}
			else
				break;
		}
		else if (digitCheck(word))
		{
			if ((++it) != words.end())
			{
				words.erase(words.begin());
				continue;
			}
			else
				break;
		}
		else if (exclusionCheck(word))
		{
			if ((++it) != words.end())
			{
				words.erase(words.begin());
				continue;
			}
			else
				return "";
		}
		else if (word.find("'s") != std::string::npos || word.find(".com") != string::npos)
		{
			if ((++it) != words.end())
			{
				words.erase(words.begin());
				continue;
			}
			else
				return "";
		}
		else if (word.find('/') != std::string::npos)
		{
			if ((++it) != words.end())
			{
				words.erase(words.begin());
				continue;
			}
			else
				return "";
		}
		else if (word.find('.') != std::string::npos)
		{
			string::iterator i = word.end() - 1;
			if (*i == '.')
			{
				for (string::iterator j = word.begin(); j != word.end() - 1; ++j)
				{
					if (*j == '.')
					{
						if ((++it) != words.end())
						{
							words.erase(words.begin());
							word = "";
							break;
						}
					}
				}
				break;
			}

			if ((++it) != words.end())
			{
				words.erase(words.begin());
				continue;
			}
			else
				return "";
		}
		else
			break;
	}
	word.erase(remove_if(word.begin(), word.end(), punctCheck), word.end());

	if (!words.empty())
	{
		words.erase(words.begin());
	}

	return(word);
}

//****************************************************************************************
//
//	DocumentFile::LoadExclusions
//
//****************************************************************************************
bool	DocumentFile::LoadExclusions(const string& name)
{
	//************************************************************************************
	//	LOCAL DATA
	bool	success;

	string word;

	string text;

	//************************************************************************************
	//	EXECUTABLE STATEMENTS
	myFile.open(name, ios::in);

	if (!myFile.fail())
	{
		while (GetLine(myFile, text))
		{
			stringstream ss(text);
			while (ss >> word)
			{
				exclusionWords.push_back(word);
			}
		}
		success = true;
	}
	else
	{
		success = false;
	}
	myFile.close();
	myFile.clear();
	return(success);
}

//****************************************************************************************
//
//	DocumentFile::Open
//
//****************************************************************************************
bool	DocumentFile::Open(const string& name)
{
	//************************************************************************************
	//	LOCAL DATA

	//************************************************************************************
	//	EXECUTABLE STATEMENTS
	file_.open(name, ios::in);
	myOtherFile.open(name, ios::in);
	anotherFile.open(name, ios::in);

	if (!file_.fail())
	{
		//	You may add any useful initialization here.
		return(true);
	}
	else
	{
		return(false);
	}
}

//****************************************************************************************
//
//	DocumentFile::Read
//
//****************************************************************************************
bool	DocumentFile::Read()
{
	//************************************************************************************
	//	LOCAL DATA
	bool	success;
	string word;
	string::iterator s;
	//************************************************************************************
	//	EXECUTABLE STATEMENTS
	text_.clear();

	if (GetLine(file_, text_))
		success = true;
	else
		success = false;
	if (text_ == "")
	{
		if (GetLine(file_, text_))
		{
			if (text_ == "")
			{
				GetLine(file_, text_);
				pageNumber_++;
			}
		}
	}

	stringstream ss(text_);
	while (ss >> word)
	{
		words.push_back(word);
		wordsForIndex.push_back(word);
		wordsForSearch.push_back(word);
	}

	return(success);
}

//****************************************************************************************
//
//	DocumentIndex::Create
//
//****************************************************************************************
void	DocumentIndex::Create(DocumentFile& documentFile)
{
	//************************************************************************************
	//	LOCAL DATA
	string check;
	vector<string> excessWords;

	//************************************************************************************
	//	EXECUTABLE STATEMENTS

	documentFile.ReadAll();
	documentFile.pageNumber_ = 1;
	while (documentFile.Read())
	{
		while (true)
		{
			check = documentFile.GetWord();

			if (find(excessWords.begin(), excessWords.end(), check) != excessWords.end())
			{
				continue;
			}
			if (check == "")
			{
				break;
			}
			if (check != "" && !documentFile.digitCheck(check))
			{
				if (!wordCount(documentFile, check))
				{
					pageNums.clear();
					pageNums.insert(documentFile.GetPageNumber());
					search(documentFile, check);
					myMap.insert(make_pair(check, pageNums));
				}
				else
				{
					excessWords.push_back(check);
					continue;
				}
			}
			else
			{
				continue;
			}
		}
	}

	documentFile.Close();
	return;
}

//****************************************************************************************
//
//	DocumentIndex::Write
//
//****************************************************************************************
void	DocumentIndex::Write(ostream& indexStream)
{
	//************************************************************************************
	//	LOCAL DATA

	//************************************************************************************
	//	EXECUTABLE STATEMENTS
	for (z = myMap.begin(); z != myMap.end(); z++)
	{
		indexStream << z->first << " ";
		if (z->second.size() > 1)
		{
			for (x = z->second.begin(); x != z->second.end(); x++)
			{
				indexStream << *x;
				set<int>::iterator w = x;

				if ((++w != z->second.end()))
				{
					indexStream << ", ";

				}
			}
			indexStream << endl;
		}
		else
		{
			for (x = z->second.begin(); x != z->second.end(); x++)
			{
				indexStream << *x;
			}
			indexStream << endl;
		}
	}
	return;
}

void	DocumentIndex::search(DocumentFile& documentFile, string word)
{

	string check;
	string checkWords;
	documentFile.pageNum = 1;
	documentFile.anotherFile.clear();
	documentFile.anotherFile.seekg(0, ios::beg);

	while (GetLine(documentFile.anotherFile, checkWords))
	{

		stringstream ss(checkWords);
		if (checkWords == "")
		{
			if (GetLine(documentFile.anotherFile, checkWords))
			{
				stringstream s(checkWords);
				if (checkWords == "");
				{
					++documentFile.pageNum;
				}
			}
			else
			{
				break;
			}
		}
		while (ss >> check)
		{

			string::iterator i = check.end() - 1;
			if (ispunct(*i))
			{
				check.erase(remove_if(check.begin(), check.end(), punctCheck), check.end());
			}

			if (check == word)
			{
				int num = documentFile.getPageNum();
				auto seek = pageNums.find(num);
				if (seek != pageNums.end())
				{
					continue;
				}
				else
				{
					pageNums.insert(num);
				}
			}
		}
	}
}

int		DocumentFile::getPageNum()
{
	return(pageNum);
}

bool	DocumentIndex::wordCount(DocumentFile& documentFile, string word)
{
	count = 0;

	for (vector<string>::iterator i = begin(documentFile.readAll); i != end(documentFile.readAll); ++i)
	{
		if (word == *i)
		{
			++count;
		}
		if (count > 10)
		{
			return(true);
		}
	}

	return(false);
}

bool	DocumentFile::hyphenCheck(string word)
{

	for (string::iterator s = word.begin(); s != word.end(); s++)
	{
		if (*s == '-')
		{
			return(true);
		}
	}

	return(false);
}

bool	DocumentFile::digitCheck(string word)
{

	for (string::iterator s = word.begin(); s != word.end(); s++)
	{
		if (isdigit(*s))
		{
			return(true);
		}
	}

	return(false);
}

bool	DocumentFile::exclusionCheck(string word)
{
	bool flag = false;

	for (i = exclusionWords.begin(); i != exclusionWords.end(); i++)
	{
		if (word != *i)
		{
			flag = false;
		}
		else
		{
			flag = true;
			break;
		}
	}

	return(flag);
}

bool	DocumentFile::read()
{
	//************************************************************************************
	//	LOCAL DATA
	bool	success;
	string word;
	string::iterator s;

	//************************************************************************************
	//	EXECUTABLE STATEMENTS
	text_.clear();

	if (getline(myOtherFile, text_))
		success = true;
	else
		success = false;
	if (text_ == "")
	{
		if (GetLine(myOtherFile, text_))
		{
			if (text_ == "")
			{
				GetLine(myOtherFile, text_);
				pageNumber_++;
			}
		}
	}

	text_.erase(remove_if(text_.begin(), text_.end(), punctCheck), text_.end());

	stringstream ss(text_);
	while (ss >> word)
	{
		wordsForSearch.push_back(word);
	}
	return(success);
}

void	DocumentFile::ReadAll()
{
	//************************************************************************************
	//	LOCAL DATA
	bool	success;
	string word;
	string::iterator s;

	//************************************************************************************
	//	EXECUTABLE STATEMENTS

	allText.clear();
	myOtherFile.clear();
	myOtherFile.seekg(0, ios::beg);

	while (GetLine(myOtherFile, allText))
	{
		if (allText.find('.') != std::string::npos)
		{
			string::iterator i = allText.end() - 1;
			if (ispunct(*i))
			{
				allText.erase(remove_if(allText.begin(), allText.end(), punctCheck), allText.end());
			}
		}
		stringstream ss(allText);
		while (ss >> word)
		{
			readAll.push_back(word);
		}
	}

}