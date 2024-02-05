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

// * implementation for classes in Commands.h

// ? Command

Command::Command(const char *cmd_line)
    : m_operands(),
      m_cmd_line(cmd_line),
      m_run_mode(_isBackgroundCommand(cmd_line) ? (RunType::BackGround) : (RunType::ForeGround))
{
  std::stringstream ss(m_cmd_line);
  for (std::string temp_str; ss >> temp_str;)
  {
    m_operands.push_back(temp_str);
  }

  if (m_operands.size() == 0)
  {
    // TODO: what to do in case of an empty command (all spaces)
  }
}

unsigned int Command::numOfArguments() const
{
  // TODO : m_operands.size() == 0 ? what
  return m_operands.size() - 1;
}

// ? BuiltInCommand

BuiltInCommand::BuiltInCommand(const char *cmd_line)
    : Command(cmd_line)
{
  // TODO : _removeBackgroundSign(m_cmd_line);
}

// ? ExternalCommand

ExternalCommand::ExternalCommand(const char *cmd_line)
    : Command(cmd_line)
{
}

void ExternalCommand::execute()
{
}

// ? PipeCommand

PipeCommand::PipeCommand(const char *cmd_line)
    : Command(cmd_line)
{
}

void PipeCommand::execute()
{
}

// ? RedirectionCommand

RedirectionCommand::RedirectionCommand(const char *cmd_line)
    : Command(cmd_line)
{
}

void RedirectionCommand::execute()
{
}

// ? ChangePromptCommand

ChangePromptCommand::ChangePromptCommand(const char *cmd_line)
    : BuiltInCommand(cmd_line)
{
  // TODO piazza: ask for name validity
}

void ChangePromptCommand::execute()
{
  SmallShell::getInstance().setPrompt(
      (m_operands.size() == 1) ? SmallShell::DEFAULT_PROMPT : m_operands.front());
}

// ? ShowPidCommand

ShowPidCommand::ShowPidCommand(const char *cmd_line)
    : BuiltInCommand(cmd_line)
{
}

void ShowPidCommand::execute()
{
  // TODO: getpid() fails?
  std::cout << "smash pid is " << getpid() << '\n';
}

// ? GetCurrDirCommand

GetCurrDirCommand::GetCurrDirCommand(const char *cmd_line)
    : BuiltInCommand(cmd_line)

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

ChangeDirCommand::ChangeDirCommand(const char *cmd_line, char **plastPwd)
    : BuiltInCommand(cmd_line)
{
  if (m_operands.size() > 2) // more than the command name and the only argument
  {
    std::cerr << "smash error: cd: too many arguments\n";
  }
  // has one argument exactly (no arguments will not be tested)
}

void ChangeDirCommand::execute()
{
  // TODO recheck the whole edge cases

  if (m_operands.back().compare("-") == 0)
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
  else if (m_operands.back().compare("..") == 0)
  {
    // ! check if it the root dir
    if (m_operands.back().compare("/") == 0)
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

    chdir(m_operands.back().c_str());
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
  for (std::list<JobEntry>::iterator it = m_jobsList.begin(); it < m_jobsList.end(); ++it)
  {
    if ((*it).m_isStopped)
    {
      m_jobsList.erase(it);
    }
  }
}

JobsList::JobEntry *JobsList::getJobById(int jobId)
{
  for (const JobEntry &job : m_jobsList)
  {
    if (job.m_id == jobId)
    {
      return job;
    }
  }
  // TODO: what to return here??
}

void JobsList::removeJobById(int jobId)
{
  for (std::list<JobEntry>::iterator it = m_jobsList.begin(); it < m_jobsList.end(); ++it)
  {
    if ((*it).m_id == jobId)
    {
      m_jobsList.erase(it);
    }
  }
}

JobsList::JobEntry *JobsList::getLastJob()
{
  return m_jobsList.back();
}

JobsList::JobEntry *JobsList::getLastStoppedJob()
{
  for (std::list<JobEntry>::iterator it = m_jobsList.end(); it < m_jobsList.begin(); --it)
  {
    if ((*id).isStopped)
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
JobsCommand::JobsCommand(const char *cmd_line, const JobsList &jobs) // ! I changed it from * to &
    : BuiltInCommand(cmd_line), m_jobs(jobs) 
{
}

void JobsCommand::execute()
{
  m_jobs.printJobsList();
}

// ? ForegroundCommand
ForegroundCommand::ForegroundCommand(const char *cmd_line, JobsList &jobs) // ! I changed it from * to &
    : BuiltInCommand(cmd_line), m_jobs(jobs)
{
  if(numOfArguments() > 1 || std::stoi(m_operands.back()) == EOF)
  {
    std::cerr << "smash error: fg: invalid arguments\n";
  }
  else if(numOfArguments() == 0 && m_jobs.isEmpty())
  {
    std::cerr << "smash error: fg: jobs list is empty\n";
  }
}

void ForegroundCommand::execute()
{
  int id = std::stoi(m_operands.back());
  for(JobsList::JobEntry& job : m_jobs.m_jobsList)
  {
    if(job.m_id == id)
    {
      // TODO: bring to fg
      m_jobs.removeJobById(id);
      return;
    }
  }
  std::cerr << "smash error: fg: job-id " << id << " does not exist\n"; // TODO: does it need to be like this <id>
}

// ? QuitCommand
QuitCommand::QuitCommand(const char *cmd_line, JobsList &jobs)
    : BuiltInCommand(cmd_line), m_jobs(jobs)
{
}

void QuitCommand::execute()
{
  if(numOfArguments() > 0)
  {
    std::list<std::string>::iterator firstArgument = m_operands.begin();
    firstArgument++;
    if(*firstArgument == "kill")
    {
      std::cout << "smash: sending SIGKILL signal to " << m_jobs.size() << "jobs:\n";
      for(JobsList::JobEntry& job : m_jobs.m_jobsList)
      {
        std::cout << "PID:" /*TODO: change PID to print the pid of the job*/ << job.m_cmd_line;
      }
      m_jobs.killAllJobs();
    }
  }

  // TODO: exit the smash
}

// ? KillCommand
KillCommand::KillCommand(const char *cmd_line, JobsList &jobs)
{

}

void KillCommand::execute()
{

}

/*
 * External Commands
 */

/*
 * Special Commands
 */

// ? ChmodCommand

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

/**
 * Creates and returns a pointer to Command class which matches the given command line (cmd_line)
 */
Command *SmallShell::CreateCommand(const char *cmd_line)
{
  // For example:
  /*
    string cmd_s = _trim(string(cmd_line));
    string firstWord = cmd_s.substr(0, cmd_s.find_first_of(" \n"));

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
