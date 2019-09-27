//Jonathon Sherrouse, CDA3101:Section5
#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sstream> //to parse strings so we can get each word in the string and ignore spaces
#include <cctype>
#include <bitset>

using namespace std;

vector<string> storeLines(ifstream&);
bool checkIfTooLong(vector<string>);
void openInputFile(ifstream &, string);
void openOutputFile(ofstream&, string);
int binaryToDecimal(string);
bool isInstruction(string);
bool checkLabelLength(string);
bool checkIfLabelDefined(string, vector<string>);
void addLabelsToTable(vector<string>, vector<pair<string,int>>&, int&);
void addRestToTable(vector<string>, vector<pair<string,int>>&, int);
void addStringToTable(string, vector<pair<string,int>>&, vector<string>);
bool inTable(string, vector<pair<string,int>>&);
int whereTheSymbolIs(string, vector<string>);
string convertToOpcode(string, stringstream&,vector<pair<string,int>>&, int);
string decimalToBinary(string);
int returnAddress(string, vector<pair<string,int>>&);
bool isLabel(string, vector<pair<string,int>>&, int);
bool checkOffsets(vector<pair<string,int>>&, vector<string>, int);
bool isNumber(string);
bool isRegister(string);
void checkValidRegisters(vector<string>, vector<pair<string,int>>&, int numLabels);

const int MAX_LENGTH = 1000;

int main(int argc, char *argv[])
{
  if(argc != 3)
  {
    cout << "Number of arguments incorrect! Exiting program.";
    exit(1);
  }

  string outputFile = argv[2];
  string inputFile = argv[1];
  ofstream output;
  ifstream input;
  vector<string> theCode;
  vector<int> theOutput;
  vector<pair<string, int>> symbolTable;  //use this to store the symbols plus their addresses using string.find() with .fill and the symbol name
  string binary;
  int numLabels = 0;
////////////////////////////////////////
  openInputFile(input, inputFile);
  openOutputFile(output, outputFile);
////////////////////////////////////////
  theCode = storeLines(input);  //vector containing all of the code, separated line by line (element by element)

  if(checkIfTooLong(theCode) == false)  //checking to see if exceeds 1000 on each line
  {
    cout << "Lines of code too long. Exiting program.";
    exit(1);
  }
//error checking labels and adding the labels to our symbol table
addLabelsToTable(theCode, symbolTable, numLabels);
addRestToTable(theCode, symbolTable, numLabels);
checkValidRegisters(theCode, symbolTable, numLabels);

if(!(checkOffsets(symbolTable, theCode, numLabels)))
{
  cout << "Offset does not fit in 16 bits. Exiting program.";
  exit(1);
}
//it ignores all the white space for some reason
string tester;
string line;
//after error checking
for(int i = 0; i < theCode.size(); i++) //going through each line of code
{
  stringstream stream(theCode[i]);
  stream >> tester;

  if(theCode[i].find(".fill") != string::npos)
  {
    stream >> tester;
    stream >> tester;


    if(isLabel(tester, symbolTable, numLabels))
    {
      output << returnAddress(tester, symbolTable) << endl;
    }
    else
    {
      output << tester << endl;
    }
  }

  else if(isLabel(tester, symbolTable, numLabels))
  {
    stream >> tester;
    if(tester == "halt")
    {
      output << "25165824" << endl;
    }
    else if(tester == "noop")
    {
      output << "29360128" << endl;
    }
    else
    {
      line = convertToOpcode(tester, stream, symbolTable, i);
      output << binaryToDecimal(line) << endl;
    }
  }
  else
  {
    if(tester == "halt")
    {
      output << "25165824" << endl;
    }
    else if(tester == "noop")
    {
      output << "29360128" << endl;
    }
    else
    {
      line = convertToOpcode(tester, stream, symbolTable, i);
      output << binaryToDecimal(line) << endl;
    }
  }
}

}
////////////////////////////////////////////////////////////////////////////////////////////////
vector<string> storeLines(ifstream& file) //parsing code
{
  vector<string> theCode;
  string eachLine;

  while(getline(file, eachLine))
  {
    theCode.push_back(eachLine);
  }

  return theCode;
}

bool checkIfTooLong(vector<string> v)
{
  for(int i = 0; i < v.size(); i++)
  {
    if(v[i].length() > MAX_LENGTH)
    {
      return false;
    }
  }
  return true;
}
//opens files below
void openInputFile(ifstream& in, string file)
{
  in.clear();
  in.open(file);

  if (!in)
  {
    cout << "Error opening file. Exiting program.";
    exit(1);
  }
}

void openOutputFile(ofstream& out, string file)
{
  out.clear();
  out.open(file);

  if (!out)
  {
    cout << "Error opening file. Exiting program.";
    exit(1);
  }
}

int binaryToDecimal(string binary)
{
  int num = 1;
  int decimal = 0;
  for(int i = binary.length()-1; i>=0; i--)
  {
    if(binary[i] == '1')
    {
      decimal += num;
    }
    num *= 2;
  }
  return decimal;
}

bool isInstruction(string chunk)
{
  string instructions[] = {"add","nand","lw","sw","beq","cmov","halt","noop"};
  bool isIt = false;

  for(int i = 0; i < 8; i++)
  {
    if(chunk == instructions[i])
    {
      isIt = true;
    }
  }

  return isIt;
}

bool checkLabelLength(string label)
{
  if(label.length() > 6)
  {
    return false;
  }
  return true;
}
//can work for other symbolic addresses
bool checkIfLabelDefined(string label, vector<string> vec)
{
  bool defined = false;

  for(int i = 0; i < vec.size(); i++)
  {
    if(vec[i].find(label) != string::npos && vec[i].find(".fill") != string::npos)
    {
      defined = true;
    }
  }
  return defined;
}

void addLabelsToTable(vector<string> theCode, vector<pair<string,int>>& symbolTable,int& numLabels)
{
  //error checks, making sure symbols are there
  for(int i = 0; i < theCode.size(); i++)
  {
    string checking;
    stringstream checkFirst(theCode[i]);
    checkFirst >> checking;
    //break if checking is done because done is not an instuction
  //checking first token of every line to see if instruciton or label
    if(theCode[i].find(".fill") != string::npos)
    {
      break;
    }

    if(isLabel(checking, symbolTable, numLabels))
    {
      cout << "Duplicate labels not allowed. Exiting program.",
      exit(1);
    }

    else if(!(isInstruction(checking))) //took out checkIfLabelDefined()
    {
      if(!isalpha(checking[0])) //added this
      {
        cout << "Lables must begin with letter. Exiting program.";
        exit(1);
      }
      else if(checking.length() > 6)
      {
        cout << "Label too long. Exiting program.";
        exit(1);
      }
      symbolTable.push_back(make_pair(checking, i));
      numLabels++;
    }
    checking.clear();
  }
}

void addRestToTable(vector<string> theCode, vector<pair<string, int>>& symbolTable, int numLabels)
{
  string checking;

  for(int i = 0; i < theCode.size(); i++)
  {
    stringstream checkFirst(theCode[i]);
    checkFirst >> checking;

    if(theCode[i].find(".fill") != string::npos)
    {
      break;
    }
    //if not instruction and not digit, that means it's a label since
    //we already error checked the labels and it's first token in string
    if(isLabel(checking, symbolTable, numLabels))
    {
      //want to grab next 4 fields after label
      for(int p = 0; p < 4; p++)
      {
        checkFirst >> checking;
        //if not an instruction after the label, exit program
        if(p == 0)
        {
          if(!(isInstruction(checking)))
          {
            cout << "Invalid instruction. Exiting program.";
            exit(1);
          }
          else if(checking == "noop" || checking == "halt") ////LOOK AT THIS!!
          {
            break;
          }
        }
        //if it's an instruction with no parameters, break
        else if(!isNumber(checking)) //(!(isdigit(checking[0])))
        {
          if(!isLabel(checking, symbolTable, numLabels) && !checkIfLabelDefined(checking, theCode)) //added !isLabel
          {
            cout << "String constant not defined. Exiting program.";
            exit(1);
          }
          else if(!isalpha(checking[0]))
          {
            cout << "Labels must begin with a letter. Exiting program.";
            exit(1);
          }
          else
          {
            addStringToTable(checking, symbolTable, theCode);
          }
        }
        checking.clear();
      }
    }
      //same thing as above, expect we grab next 3 fields instead
      else
      {
        if(theCode[i].find(".fill") != string::npos)
        {
          break;
        }
        if(!(isInstruction(checking)))
        {
          cout << "Invalid instruction. Exiting program.";
          exit(1);
        }
        for(int p = 0; p < 3; p++)
        {
          checkFirst >> checking;
          //if not an instruction after the label, exit program
          //if it's an instruction with no parameters, break
          if(p == 0)
          {
            if(checking == "noop" || checking == "halt")
            {
              break;
            }
          }
          else if(!isNumber(checking))
          {
            if(!isLabel(checking, symbolTable, numLabels) && !checkIfLabelDefined(checking, theCode))
            {
              cout << "String constant not defined. Exiting program.";
              exit(1);
            }
            else
            {
                addStringToTable(checking, symbolTable, theCode);
            }
          }
          }
          checking.clear();
        }
      }
    }


  bool inTable(string theString, vector<pair<string,int>>& vec)
  {
    for(int i = 0; i < vec.size(); i++)
    {
      if(theString == vec[i].first)
      {
        return true;
      }
    }
    return false;
  }
//////error here, cant be 0
/////add showed up here once
  void addStringToTable(string theString, vector<pair<string,int>>& symbolTable, vector<string> theCode)
  {
    if(!isInstruction(theString) && !inTable(theString, symbolTable))
    {
      int address = whereTheSymbolIs(theString, theCode);
      symbolTable.push_back(make_pair(theString, address));
    }
  }

  int whereTheSymbolIs(string theString, vector<string> theCode)
  {
    string checking;

    for(int i = 0; i < theCode.size(); i++)
    {
      if(theCode[i].find(theString) != string::npos && theCode[i].find(".fill") != string::npos)
      {
        return i;
      }
    }
  }

string decimalToBinary(string number)
{
  int num = stoi(number);
  string returnVal;
  if(num <= 7)
  {
    returnVal = bitset<3>(num).to_string();
  }
  else if(num <= 15)
  {
    returnVal = bitset<4>(num).to_string();
  }
  else if(num <= 31)
  {
    returnVal = bitset<5>(num).to_string();
  }
  else if(num <= 63)
  {
    returnVal = bitset<6>(num).to_string();
  }

  return returnVal;
}

int returnAddress(string check, vector<pair<string,int>>& symbol)
{
  for(int i = 0; i < symbol.size(); i++)
  {
    if(check == symbol[i].first)
    {
      return symbol[i].second;
    }
  }
}

///////////////STILL IN PROGRESS/////////
  string convertToOpcode(string instruction, stringstream& theStream, vector<pair<string,int>>& symbols, int currentAddress)
  {
    string opcode = "";
    string token;
    int num;
    int address;
    int lastNum;

    if(instruction == "add")
    {
      opcode += "000";
      theStream >> token;
      if(!isNumber(token))
      {
        address = returnAddress(token, symbols);
        opcode += decimalToBinary(to_string(returnAddress(token,symbols)));
      }

      else
      {
        opcode += decimalToBinary(token);
      }
      theStream >> token;
      if(!isNumber(token))
      {
        opcode += decimalToBinary(to_string(returnAddress(token,symbols)));
      }
      else
      {
        opcode += decimalToBinary(token);
      }
      theStream >> token;
      if(!isdigit(token[0]))
      {
        opcode += bitset<16>(decimalToBinary(to_string(returnAddress(token,symbols)))).to_string();
      }
      else
      {
        lastNum = stoi(token);
        opcode += bitset<16>(lastNum).to_string();
      }
      theStream.str("");
    }
    else if(instruction == "nand")
    {
      opcode += "001";
      theStream >> token;
      if(!isdigit(token[0]))
      {
        opcode += decimalToBinary(to_string(returnAddress(token,symbols)));
      }
      else
      {
        opcode += decimalToBinary(token);
      }
      theStream >> token;
      if(!isdigit(token[0]))
      {
        opcode += decimalToBinary(to_string(returnAddress(token,symbols)));
      }
      else
      {
        opcode += decimalToBinary(token);
      }
      theStream >> token;
      if(!isdigit(token[0]))
      {
        opcode += bitset<16>(decimalToBinary(to_string(returnAddress(token,symbols)))).to_string();
      }
      else
      {
        lastNum = stoi(token);
        opcode += bitset<16>(lastNum).to_string();
      }

    }
    else if(instruction == "lw")
    {
      opcode = "010";
      theStream >> token;

      if(!isdigit(token[0]))
      {
        opcode += decimalToBinary(to_string(returnAddress(token,symbols)));
      }
      else
      {
        opcode += decimalToBinary(token);
      }
      theStream >> token;
      if(!isdigit(token[0]))
      {
        opcode += decimalToBinary(to_string(returnAddress(token,symbols)));
      }
      else
      {
        opcode += decimalToBinary(token);
      }
      theStream >> token;
      if(!isNumber(token))
      {
        opcode += bitset<16>(decimalToBinary(to_string(returnAddress(token,symbols)))).to_string();
      }
      else
      {
        lastNum = stoi(token);
        opcode += bitset<16>(lastNum).to_string();
      }
      theStream.str("");
    }
    else if(instruction == "sw")
    {
      opcode += "011";
      theStream >> token;
      if(!isdigit(token[0]))
      {
        opcode += decimalToBinary(to_string(returnAddress(token,symbols)));
      }
      else
      {
        opcode += decimalToBinary(token);
      }
      theStream >> token;
      if(!isdigit(token[0]))
      {
        opcode += decimalToBinary(to_string(returnAddress(token,symbols)));
      }
      else
      {
        opcode += decimalToBinary(token);
      }
      theStream >> token;
      if(!isNumber(token))
      {
        opcode += bitset<16>(decimalToBinary(to_string(returnAddress(token,symbols)))).to_string();
      }
      else
      {
        lastNum = stoi(token);
        opcode += bitset<16>(lastNum).to_string();
      }
      theStream.str("");
    }
    else if(instruction == "beq")
    {
      opcode += "100";
      theStream >> token;
      if(!isdigit(token[0]))
      {
        opcode += decimalToBinary(to_string(returnAddress(token,symbols)));
      }
      else
      {
        opcode += decimalToBinary(token);
      }
      theStream >> token;
      if(!isdigit(token[0]))
      {
        opcode += decimalToBinary(to_string(returnAddress(token,symbols)));
      }
      else
      {
        opcode += decimalToBinary(token);
      }
      theStream >> token;
      if(!isdigit(token[0]))
      {
        int symbolAddress = returnAddress(token,symbols);
        int offset;
        if(currentAddress > symbolAddress)
        {
          offset = (((currentAddress - symbolAddress)+1)*-1);
        }
        else
        {
          offset = ((currentAddress - symbolAddress)+1);
        }
        opcode += bitset<16>(offset).to_string();
      }
      else
      {
        lastNum = stoi(token);
        opcode += bitset<16>(lastNum).to_string();
      }
    }
    else if(instruction == "cmov")
    {
      opcode += "101";
      theStream >> token;
      if(!isdigit(token[0]))
      {
        opcode += decimalToBinary(to_string(returnAddress(token,symbols)));
      }
      else
      {
        opcode += decimalToBinary(token);
      }
      theStream >> token;
      if(!isdigit(token[0]))
      {
        opcode += decimalToBinary(to_string(returnAddress(token,symbols)));
      }
      else
      {
        opcode += decimalToBinary(token);
      }
      theStream >> token;
      if(!isdigit(token[0]))
      {
        opcode += bitset<16>(decimalToBinary(to_string(returnAddress(token,symbols)))).to_string();
      }
      else
      {
        lastNum = stoi(token);
        opcode += bitset<16>(lastNum).to_string();
      }
    }
    else if(instruction == "halt")
    {
      opcode += "110";
    }
    else if(instruction == "noop")
    {
      opcode += "111";
    }
    return opcode;
  }

  bool isLabel(string test, vector<pair<string,int>>& symbols, int numLabels)
  {
    for (int i = 0; i < numLabels; i++)
    {
      if(test == symbols[i].first)
      {
        return true;
      }
    }
    return false;
  }

  bool checkOffsets(vector<pair<string, int>>& symbols, vector<string> theCode, int numLabels)
  {
    string check;
    string nextInstruction;

    for(int i = 0; i < theCode.size(); i++)
    {
      stringstream stream(theCode[i]);
      stream >> check;
      stream >> nextInstruction;

      if(isLabel(check, symbols, numLabels) && (nextInstruction == "beq" || nextInstruction == "sw" || nextInstruction == "lw"))
      {
        stream >> check;
        stream >> check;
        stream >> check;

        if(inTable(check, symbols))
        {
          int symbolAddress = returnAddress(check,symbols);
          int offset;

          if(i > symbolAddress)
          {
            offset = (((i - symbolAddress)+1)*-1)*4;
          }
          else
          {
            offset = ((i - symbolAddress)+1)*4;
          }

          if(offset < -32768 || offset > 32767)
          {
            return false;
          }
        }
        else
        {
          if(stoi(check) < -32768 || stoi(check) > 32767)
          {
            return false;
          }
        }
      }
      else if(isInstruction(check) && (check == "beq" || check == "sw" || check == "lw"))
      {
        stream >> check;
        stream >> check;

        if(inTable(check, symbols))
        {
          int symbolAddress = returnAddress(check,symbols);
          int offset;

          if(i > symbolAddress)
          {
            offset = (((i - symbolAddress)+1)*-1)*4;
          }
          else
          {
            offset = ((i - symbolAddress)+1)*4;
          }

          if(offset < -32768 || offset > 32767)
          {
            return false;
          }
        }
        else
        {
          if(stoi(check) < -32768 || stoi(check) > 32767)
          {
            return false;
          }
        }
      }
  }
  return true;
}

bool isNumber(string num)
{
  if(num[0] == '-' && num.length() > 1)
  {
    for(int i = 1; i < num.length(); i++)
    {
      if(!isdigit(num[i]))
      {
        return false;
      }
    }
  }
  else if(!isdigit(num[0]) && num[0] != '-')
  {
    return false;
  }
  else if(isdigit(num[0]))
  {
    for (int i = 0; i < num.length(); i++)
    {
      if(!isdigit(num[i]))
      {
        return false;
      }
    }
  }
  return true;
}

bool isRegister(string reg)
{
  if(!(reg == "0" || reg == "1" || reg == "2" || reg == "3" || reg == "4" || reg == "5" || reg == "6" || reg == "7"))
  {
    return false;
  }
  else{
    return true;
  }
}

void checkValidRegisters(vector<string> theCode, vector<pair<string,int>>& symbols, int numLabels)
{
  string check;
  for(int i = 0; i < theCode.size(); i++)
  {
    stringstream theStream(theCode[i]);
    theStream >> check;

    if(isLabel(check, symbols, numLabels))
    {
      theStream >> check;
    }

    if(check == "add")
    {
      theStream >> check;
      if(!isRegister(check))
      {
        cout << "Invalid argument for add. Exiting program.";
        exit(1);
      }
      theStream >> check;
      if(!isRegister(check))
      {
        cout << "Invalid argument for add. Exiting program.";
        exit(1);
      }
      theStream >> check;
      if(!isRegister(check))
      {
        cout << "Invalid destination register for add. Exiting program.";
        exit(1);
      }
    }
    else if(check == "nand")
    {
      theStream >> check;
      if(!isRegister(check))
      {
        cout <<"Invalid argument for nand. Exiting program.";
        exit(1);
      }
      theStream >> check;
      if(!isRegister(check))
      {
        cout <<"Invalid argument for nand. Exiting program.";
        exit(1);
      }
      theStream >> check;
      if(!isRegister(check))
      {
        cout << "Invalid destination register for nand. Exiting program.";
        exit(1);
      }
    }
    else if(check == "cmov")
    {
      theStream >> check;
      if(!isRegister(check))
      {
        cout << "Invalid argument for cmov. Exiting program.";
        exit(1);
      }
      theStream >> check;
      if(!isRegister(check))
      {
        cout <<"Invalid argument for cmov. Exiting program.";
        exit(1);
      }
      theStream >> check;
      if(!isRegister(check))
      {
        cout << "Invalid destination register for cmov. Exiting program.";
        exit(1);
      }
    }
    else if(check == "lw")
    {
      theStream >> check;

      if(!isRegister(check))
      {
        cout << "Invalid argument for lw. Exiting program.";
        exit(1);
      }
      theStream >> check;
      if(!isRegister(check))
      {
        cout << "Invalid argument for lw. Exiting program.";
        exit(1);
      }
      theStream >> check;
      if(!inTable(check, symbols) && !isNumber(check))
      {
        cout << "Invalid offset argument for lw. Exiting program.";
        exit(1);
      }
    }
    else if(check == "sw")
    {
      theStream >> check;
      if(!isRegister(check))
      {
        cout << "Invalid argument for sw. Exiting program.";
        exit(1);
      }
      theStream >> check;
      if(!isRegister(check))
      {
        cout << "Invalid argument for sw. Exiting program.";
        exit(1);
      }
      theStream >> check;
      if(!inTable(check, symbols) && !isNumber(check))
      {
        cout << "Invalid offset argument for sw. Exiting program.";
        exit(1);
      }
    }
    else if(check == "beq")
    {
      theStream >> check;
      if(!isRegister(check))
      {
        cout << "Invalid argument for beq. Exiting program.";
        exit(1);
      }
      theStream >> check;
      if(!isRegister(check))
      {
        cout << "Invalid argument for beq. Exiting program.";
        exit(1);
      }
      theStream >> check;
      if(!inTable(check, symbols) && !isNumber(check))
      {
        cout << "Invalid offset argument for beq. Exiting program.";
        exit(1);
      }
    }
  }
}
