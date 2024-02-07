#include <unistd.h>
#include <string.h>
#include <iostream>
#include <vector>
#include <sstream>
#include <sys/wait.h>
#include <iomanip>
#include "Commands.h"

using namespace std;

const std::string WHITESPACE = " \n\r\t\f\v";

#if 0
#define FUNC_ENTRY() \
  cout << __PRETTY_FUNCTION__ << " --> " << endl;

#define FUNC_EXIT() \
  cout << __PRETTY_FUNCTION__ << " <-- " << endl;
#else
#define FUNC_ENTRY()
#define FUNC_EXIT()
#endif

string _ltrim(const std::string &s)
{
  size_t start = s.find_first_not_of(WHITESPACE);
  return (start == std::string::npos) ? "" : s.substr(start);
}

string _rtrim(const std::string &s)
{
  size_t end = s.find_last_not_of(WHITESPACE);
  return (end == std::string::npos) ? "" : s.substr(0, end + 1);
}

string _trim(const std::string &s)
{
  return _rtrim(_ltrim(s));
}

int _parseCommandLine(const char *cmd_line, char **args)
{
  FUNC_ENTRY()
  int i = 0;
  std::istringstream iss(_trim(string(cmd_line)).c_str());
  for (std::string s; iss >> s;)
  {
    args[i] = (char *)malloc(s.length() + 1);
    memset(args[i], 0, s.length() + 1);
    strcpy(args[i], s.c_str());
    args[++i] = NULL;
  }
  return i;

  FUNC_EXIT()
}

bool _isBackgroundCommand(const char *cmd_line)
{
  const string str(cmd_line);
  return str[str.find_last_not_of(WHITESPACE)] == '&';
}

void _removeBackgroundSign(char *cmd_line)
{
  const string str(cmd_line);
  // find last character other than spaces
  unsigned int idx = str.find_last_not_of(WHITESPACE);
  // if all characters are spaces then return
  if (idx == string::npos)
  {
    return;
  }
  // if the command line does not end with & then return
  if (cmd_line[idx] != '&')
  {
    return;
  }
  // replace the & (background sign) with space and then remove all tailing spaces.
  cmd_line[idx] = ' ';
  // truncate the command line string up to the last non-space character
  cmd_line[str.find_last_not_of(WHITESPACE, idx) + 1] = 0;
}

//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//
//-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-//

// * implementation for classes in Commands.h

class CommandBuilder
{
  std::string m_name;
  std::vector<std::string> m_operands;
  std::vector<std::string> m_args1;
  int m_flag; // TODO: better name?
  int m_numOfOperands;

  std::vector<std::string> m_args2;

  bool m_isBackgroundCommand;
  CommandType m_type;

public:
  CommandBuilder(){};
  ~CommandBuilder(){};

  void addOperand(const std::string &op)
  {
    m_operands.push_back(op);
    if (m_type == CommandType::BuiltIn || m_type == CommandType::TimeOut)
    {
      if (m_numOfOperands == 0)
      {
        m_name = op;
      }
      else
      {
        m_args1.push_back(op);
      }
    }
    else if (m_type == CommandType::Pipe || m_type == CommandType::Pipe_err ||
             m_type == CommandType::IO_Append || m_type == CommandType::IO_Override)
    {
      if (op == "|" || op == "|&")
      {
        m_name = "pipe";
        m_flag = 1;
      }
      else if (op == ">" || op == ">>")
      {
        m_name = "io";
        m_flag = 1;
      }
      if (m_flag == 0)
      {
        m_args1.push_back(op);
      }
      else if (m_flag == 1)
      {
        m_args2.push_back(op);
      }
    }
    else if (m_type == CommandType::External)
    {
      if (m_numOfOperands == 0)
      {
        m_name = op;
      }
      else
      {
        m_args1.push_back(op);
      }
    }
    m_numOfOperands++;
  }

  Command *build()
  {
    if (m_name == "chprompt")
    {
      return new ChangePromptCommand(m_operands, m_args1);
    }
    else if (m_name == "showpid")
    {
      return new ShowPidCommand(m_operands, m_args1);
    }
    else if (m_name == "pwd")
    {
      return new GetCurrDirCommand(m_operands, m_args1);
    }
    else if (m_name == "cd")
    {
      return new ChangeDirCommand(m_operands, m_args1);
    }
    else if (m_name == "jobs")
    {
      return new JobsCommand(m_operands, m_args1);
    }
    else if (m_name == "fg")
    {
      return new ForegroundCommand(m_operands, m_args1);
    }
    else if (m_name == "quit")
    {
      return new QuitCommand(m_operands, m_args1);
    }
    else if (m_name == "kill")
    {
      return new KillCommand(m_operands, m_args1);
    }
    else if (m_name == "chmod")
    {
      return new ChmodCommand(m_operands, m_args1);
    }
    else if (m_name == "pipe")
    {
      return new PipeCommand(m_operands, m_args1);
    }
    else if (m_name == "io")
    {
      return new RedirectionCommand(m_operands, m_args1);
    }
    // else if(m_name == "timeout")
    //{
    //   return new TimeOutCommand(m_operands, m_args1);
    //}
    return nullptr;
  }


  void setBackground(bool isBackground)
  {
    m_isBackgroundCommand = isBackground;
  }

  void setType(CommandType type)
  {
    m_type = type;
  }
}
;

class CommandParser
{

public:
  CommandParser(){};
  ~CommandParser(){};
  Command *parse(const std::string &cmd_line)
  {
    std::stringstream ss(cmd_line);
    CommandBuilder cb;
    cb.setBackground(_isBackgroundCommand(cmd_line.c_str()));
    cb.setType(getType(cmd_line));

    for (std::string tmp; ss >> tmp;)
    {
      cb.addOperand(tmp);
    }

    return cb.build();
  }

  CommandType getType(const std::string &cmd_line) const
  {
    std::stringstream ss(cmd_line);
    for (std::string tmp; ss >> tmp;)
    {
      if (tmp == ">")
      {
        return CommandType::IO_Override;
      }
      else if (tmp == ">>")
      {
        return CommandType::IO_Append;
      }
      else if (tmp == "|")
      {
        return CommandType::Pipe;
      }
      else if (tmp == "|&")
      {
        return CommandType::Pipe_err;
      }
      else if (tmp == "timeout")
      {
        return CommandType::TimeOut;
      }
      else if (tmp == "chprompt" || tmp == "showpid" || tmp == "pwd" ||
               tmp == "cd" || tmp == "jobs" || tmp == "fg" || tmp == "quit" || tmp == "kill" || tmp == "chmod")
      {
        return CommandType::BuiltIn;
      }
    }
    return CommandType::External;
  }
};

// ? Command

Command::Command(const std::string name, std::vector<std::string> operands, CommandType type, bool isBackground)
    : m_name(name), m_operands(operands), m_type(type)
{
  m_run_mode = (isBackground == true) ? RunType::BackGround : RunType::ForeGround;
}

const std::string &Command::getCMDline() const
{
  std::string cmdLine;
  for (auto op : m_operands)
  {
    cmdLine += " " + op;
  }
  return cmdLine;
}

// ? BuiltInCommand

BuiltInCommand::BuiltInCommand(const std::string name, std::vector<std::string> operands, std::vector<std::string> args)
    : Command(name, operands, CommandType::BuiltIn, false), m_args(args)
{
}

// ? ExternalCommand

ExternalCommand::ExternalCommand(const std::string name, std::vector<std::string> operands, std::vector<std::string> args, bool isBackground)
    : Command(name, operands, CommandType::External, isBackground), m_args(args)
{
}

void ExternalCommand::execute()
{
}

// ? PipeCommand

PipeCommand::PipeCommand(const std::string name, std::vector<std::string> operands, std::vector<std::string> args1, std::vector<std::string> args2, CommandType type)
    : Command(name, operands, type, false), m_args1(args1), m_args2(args2)
{
}

void PipeCommand::execute()
{
}

// ? RedirectionCommand

RedirectionCommand::RedirectionCommand(const std::string name, std::vector<std::string> operands, std::vector<std::string> args1, std::vector<std::string> args2, CommandType type)
    : Command(name, operands, type, false), m_args1(args1), m_args2(args2)
{
}

void RedirectionCommand::execute()
{
}

// ? ChangePromptCommand

ChangePromptCommand::ChangePromptCommand(std::vector<std::string> operands, std::vector<std::string> args)
    : BuiltInCommand("chprompt", operands, args)
{
  // TODO piazza: ask for name validity
}

void ChangePromptCommand::execute()
{
  // if the command has only its name, then return back the default prompt
  //    otherwise,
  SmallShell::getInstance().setPrompt(
      (m_args.size() == 0) ? SmallShell::DEFAULT_PROMPT : m_args.front());
  // ! Not front, front is the command name, the next is the first arg
}

// ? ShowPidCommand

ShowPidCommand::ShowPidCommand(std::vector<std::string> operands, std::vector<std::string> args)
    : BuiltInCommand("showpid", operands, args)
{
}

void ShowPidCommand::execute()
{
  // TODO: getpid() fails?
  std::cout << "smash pid is " << getpid() << '\n';
}

// ? GetCurrDirCommand

GetCurrDirCommand::GetCurrDirCommand(std::vector<std::string> operands, std::vector<std::string> args)
    : BuiltInCommand("pwd", operands, args)

{
}

void GetCurrDirCommand::execute()
{
  char path[COMMAND_MAX_PATH_LENGTH];
  /**
   * TODO : what if getcwd fails? perror gets called?
   */
  std::cout << string(getcwd(path, COMMAND_MAX_PATH_LENGTH)) << '\n';
}

// ? ChangeDirCommand

ChangeDirCommand::ChangeDirCommand(std::vector<std::string> operands, std::vector<std::string> args) //!!!! i rmoved char** plastPwd
    : BuiltInCommand("cd", operands, args)
{
  if (m_args.size() > 1) // more than the only argument
  {
    std::cerr << "smash error: cd: too many arguments\n";
  }
  // has one argument exactly (no arguments will not be tested)
}

void ChangeDirCommand::execute()
{
  // TODO recheck the whole edge cases

  if (m_args.back().compare("-") == 0)
  {
    if (m_path_history.size() == 0)
    {
      std::cerr << "smash error: cd: OLDPWD not set\n";
    }
    else
    {
      // TODO: handle fail
      chdir(m_path_history.back().c_str());

      m_path_history.pop_back();
    }
  }
  else if (m_args.back().compare("..") == 0)
  {
    // ! check if it the root dir
    if (m_args.back().compare("/") == 0) // !!!!!!!!! i think that this is wrong
    {
      // TODO error?
    }
    else
    {
      chdir("..");
    }
  }
  else
  {
    char path[COMMAND_MAX_PATH_LENGTH];
    /**
     * TODO : what if getcwd fails? perror gets called?
     */
    m_path_history.push_back(getcwd(path, COMMAND_MAX_PATH_LENGTH));

    chdir(m_args.back().c_str());
  }
}

// ? JobsList class
void JobsList::addJob(Command *cmd, bool isStopped = false)
{
  m_jobsList.push_back(JobEntry(cmd->getCMDline(), getNextID(), isStopped));
}

void JobsList::printJobsList()
{
  for (const JobEntry &job : m_jobsList)
  {
    std::cout << "[" << job.m_id << "] " << job.m_cmd_line << "\n";
  }
}

void JobsList::killAllJobs()
{
  m_jobsList.clear();
}

void JobsList::removeFinishedJobs()
{
  for (std::list<JobEntry>::iterator it = m_jobsList.begin(); it != m_jobsList.end(); ++it)
  {
    if ((*it).m_isStopped)
    {
      m_jobsList.erase(it);
    }
  }
}

JobsList::JobEntry *JobsList::getJobById(int jobId)
{
  // for (auto JobEntry)
  for (JobEntry &job : m_jobsList)
  {
    if (job.m_id == jobId)
    {
      return &job;
    }
  }
  return nullptr;
}

void JobsList::removeJobById(int jobId)
{
  for (std::list<JobEntry>::iterator it = m_jobsList.begin(); it != m_jobsList.end(); ++it)
  {
    if ((*it).m_id == jobId)
    {
      m_jobsList.erase(it);
    }
  }
}

JobsList::JobEntry *JobsList::getLastJob()
{
  return &m_jobsList.back();
}

JobsList::JobEntry *JobsList::getLastStoppedJob()
{
  for (auto it = m_jobsList.rend(); it < m_jobsList.rbegin(); --it)
  {
    if ((*it).m_isStopped)
    {
      // TODO: what to return here??
    }
  }
  // TODO: what to return here??
}

unsigned long JobsList::getNextID() const
{
  // TODO: should i check for unfinished jobs?
  return (m_jobsList.size() == 0) ? (1) : (m_jobsList.back().m_id + 1);
}

bool JobsList::isEmpty() const
{
  return m_jobsList.empty();
}

// ? JobsCommand
JobsCommand::JobsCommand(std::vector<std::string> operands, std::vector<std::string> args, const JobsList &jobs)
    : BuiltInCommand("jobs", operands, args), m_jobs(jobs)
{
}

void JobsCommand::execute()
{
  m_jobs.printJobsList();
}

// ? ForegroundCommand
ForegroundCommand::ForegroundCommand(std::vector<std::string> operands, std::vector<std::string> args, JobsList &jobs)
    : BuiltInCommand("fg", operands, args), m_jobs(jobs)
{
  if (numOfArguments() > 1)
  {
    std::cerr << "smash error: fg: invalid arguments\n";
  }
  try
  {
    m_jobID = std::stoi(m_operands.back());
  }
  catch (const std::exception &e)
  {
    std::cerr << "smash error: fg: invalid arguments\n";
  }
  if (numOfArguments() == 0 && m_jobs.isEmpty())
  {
    std::cerr << "smash error: fg: jobs list is empty\n";
  }
}

void ForegroundCommand::execute()
{
  for (JobsList::JobEntry &job : m_jobs.m_jobsList)
  {
    if (job.m_id == m_jobID)
    {
      // TODO: bring to fg
      m_jobs.removeJobById(m_jobID);
      return;
    }
  }
  std::cerr << "smash error: fg: job-id " << m_jobID << " does not exist\n";
}

// ? QuitCommand
QuitCommand::QuitCommand(std::vector<std::string> operands, std::vector<std::string> args, JobsList &jobs)
    : BuiltInCommand("quit", operands, args), m_jobs(jobs)
{
}

void QuitCommand::execute()
{
  if (numOfArguments() > 0)
  {
    if (m_args.front() == "kill")
    {
      std::cout << "smash: sending SIGKILL signal to " << m_jobs.m_jobsList.size() << "jobs:\n";
      for (JobsList::JobEntry &job : m_jobs.m_jobsList)
      {
        std::cout << "PID:" /*TODO: change PID to print the pid of the job*/ << job.m_cmd_line;
      }
      m_jobs.killAllJobs();
    }
  }

  // TODO: exit the smash
}

// ? KillCommand
KillCommand::KillCommand(std::vector<std::string> operands, std::vector<std::string> args, JobsList &jobs)
    : BuiltInCommand("kill", operands, args), m_jobs(jobs)
{
  if (numOfArguments() > 2)
  {
    std::cout << "smash error: kill: invalid arguments\n";
  }
  try
  {
    m_sigNum = std::stoi(m_args.front());
    m_jobID = std::stoi(m_args.back());
  }
  catch (const std::exception &e)
  {
    std::cerr << "smash error: kill: invalid arguments\n";
  }
}

void KillCommand::execute()
{
  JobsList::JobEntry *job = m_jobs.getJobById(m_jobID);
  if (job != nullptr)
  {
    m_jobs.removeJobById(m_jobID);
    // TODO: kill the job
  }
}

/*
 * External Commands
 */

/*
 * Special Commands
 */

// ? ChmodCommand
ChmodCommand::ChmodCommand(std::vector<std::string> operands, std::vector<std::string> args)
    : BuiltInCommand("chmod", operands, args)
{
}

void ChmodCommand::execute()
{
  // TODO: exec
}

/*
 * Small Shell class
 */

// ? value for the static constant in SmallShell
const std::string SmallShell::DEFAULT_PROMPT = "smash";

SmallShell::SmallShell()
    : m_prompt(DEFAULT_PROMPT)
{
  // TODO: add your implementation
}

SmallShell::~SmallShell()
{
  // TODO: add your implementation
}

// std::map<std::string, Command *> = {
//     {"",
//      nullptr}};

/**
 * Creates and returns a pointer to Command class which matches the given command line (cmd_line)
 */
Command *SmallShell::CreateCommand(const char *cmd_line)
{
  std::string cmd_s = _trim(string(cmd_line));
  std::string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));
  // if a WHITESPACE string was given, then return an "empty" command
  if (firstWord.compare("") == 0)
  {
    return nullptr;
  }
  else if (firstWord.compare("chprompt") == 0)
  {
  }
  else if (firstWord.compare("showpid") == 0)
  {
  }
  else if (firstWord.compare("pwd") == 0)
  {
  }
  else if (firstWord.compare("cd") == 0)
  {
  }
  else if (firstWord.compare("jobs") == 0)
  {
  }
  else if (firstWord.compare("fg") == 0)
  {
  }
  else if (firstWord.compare("quit") == 0)
  {
  }
  else if (firstWord.compare("kill") == 0)
  {
  }
  else if (firstWord.compare("chmod") == 0)
  {
  }
  else
  {
  }

  // For example:
  /*

    if (firstWord.compare("pwd") == 0) {
      return new GetCurrDirCommand(cmd_line);
    }
    else if (firstWord.compare("showpid") == 0) {
      return new ShowPidCommand(cmd_line);
    }
    else if ...
    .....
    else {
      return new ExternalCommand(cmd_line);
    }
    */
  return nullptr;
}

void SmallShell::executeCommand(const char *cmd_line)
{
  // TODO: Add your implementation here
  /* *
   * Before Executing any command do "housekeeping" work, like:
   *    1) delete finished jobs from the jobs list.
   */

  // * 1)
  m_background_jobs.removeFinishedJobs();
  // * 2)

  // * Execute command

  // for example:
  Command *cmd = CreateCommand(cmd_line);
  // cmd->execute();
  // Please note that you must fork smash process for some commands (e.g., external commands....)
}

const std::string &SmallShell::getPrompt() const
{
  return m_prompt;
}

void SmallShell::setPrompt(const std::string &newPrompt)
{
  // TODO piazza: any validity checks for the newPrompt?
  m_prompt = newPrompt;
}
