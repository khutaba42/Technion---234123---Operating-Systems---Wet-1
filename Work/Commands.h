#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <map>  // TODO piazza: for what?
#include <list> // for JobsList

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define COMMAND_MAX_PATH_LENGTH (80)

class Command
{
protected:
  enum RunType
  {
    ForeGround,
    BackGround
  };
  // *- data members
  std::list<std::string> m_operands;
  std::string m_cmd_line;
  RunType m_run_mode;

public:
  Command(const char *cmd_line);
  virtual ~Command() = default;
  virtual void execute() = 0;
  // virtual void prepare();
  // virtual void cleanup();
  // *- extra methods as needed
  RunType getRunType() const;
  const std::string &getCMDline() const;
  unsigned int numOfArguments() const;
};

class BuiltInCommand : public Command
{
public:
  BuiltInCommand(const char *cmd_line);
  virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command
{
public:
  ExternalCommand(const char *cmd_line);
  virtual ~ExternalCommand() {}
  void execute() override;
};

class PipeCommand : public Command
{
  // *- data members
public:
  PipeCommand(const char *cmd_line);
  virtual ~PipeCommand() {}
  void execute() override;
};

class RedirectionCommand : public Command
{
  // *- data members
public:
  explicit RedirectionCommand(const char *cmd_line);
  virtual ~RedirectionCommand() {}
  void execute() override;
  // void prepare() override;
  // void cleanup() override;
};

/*
 * Built In Commands
 */

/** Command number 1:
 * @brief `chprompt` command will allow the user to change the prompt displayed by the smash while waiting for the next command.
 *    If no parameters were sent, then the prompt shall be reset to smash. If more than one parameter was sent, then the rest shall be ignored.
 *    Note that this command will not change the prompt in error messages that we will see later.
 */
class ChangePromptCommand : public BuiltInCommand
{
  // *- data members
public:
  ChangePromptCommand(const char *cmd_line);
  virtual ~ChangePromptCommand() {}
  void execute() override;
};

/** Command number 2:
 * @brief `showpid` command prints the smash pid.
 *    If any number of arguments were provided with this command then they will be ignored.
 */
class ShowPidCommand : public BuiltInCommand
{
public:
  ShowPidCommand(const char *cmd_line);
  virtual ~ShowPidCommand() {}
  void execute() override;
};

/** Command number 3:
 * @brief `pwd` command has no arguments.
 *    pwd prints the full path of the current working directory. In the next command (cd command) will explain how to change the current working directory.
 *    You may use `getcwd` system call to retrieve the current working directory.
 *    If any number of arguments were provided with pwd then they will be ignored.
 */
class GetCurrDirCommand : public BuiltInCommand
{
public:
  GetCurrDirCommand(const char *cmd_line);
  virtual ~GetCurrDirCommand() {}
  void execute() override;
};

/** Command number 4:
 * @brief Change directory (`cd`) command receives a single argument <path> that describes the relative or full path to change the current working directory to it.
 *    There is a special argument that cd can get which is “-“. If “-“ was specified as the only argument of cd command then it will change the current working directory to the last working directory.
 *    That means, if the current working directory is X, then cd was executed to change the current working directory to Y, and then cd was called (again) with “-“ then it should go back and set the current working directory to X.
 *    You may use `chdir` system call to change the current working directory.
 *
 *    If more than one argument was provided, then cd command should print the following error message:
 *        ```smash error: cd: too many arguments```
 *    If the last working directory is empty and “cd -“ was called (before calling cd with some path to change current working directory to it) then it should print the following error message:
 *        ```smash error: cd: OLDPWD not set```
 *    If `chdir()` system call fails (e.g., <path> argument points to a non-existing path) then perror should be used to print a proper error message (as described in Error Handling section).
 */
class ChangeDirCommand : public BuiltInCommand
{
  // *- data members
  static std::list<std::string> m_path_history; // default c'tor will be called

public:
  ChangeDirCommand(const char *cmd_line, char **plastPwd);
  virtual ~ChangeDirCommand() {}
  void execute() override;
};

class JobsList
{
public:
  class JobEntry
  {
  public:
    // *- data members
    //TODO check if we should prettify the command
    std::string m_cmd_line;
    unsigned long m_id;
    bool m_isStopped;
    JobEntry(const string& cmdLine, unsigned long id, bool isStopped) : m_cmd_line(cmdLine), m_id(id), m_isStopped(isStopped){};
  };
  // *- data members
  std::list<JobEntry> m_jobs;

public:
  JobsList(){};
  ~JobsList(){};
  void addJob(Command *cmd, bool isStopped = false);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry *getJobById(int jobId);
  void removeJobById(int jobId);
  JobEntry *getLastJob();
  JobEntry *getLastStoppedJob();
  // *- add extra methods or modify existing ones as needed
  unsigned long getNextID() const;
};

/** Command number 5:
 * @brief
 */
class JobsCommand : public BuiltInCommand
{
  // *- data members
public:
  JobsCommand(const char *cmd_line, JobsList *jobs);
  virtual ~JobsCommand() {}
  void execute() override;
};

/** Command number 6:
 * @brief
 */
class ForegroundCommand : public BuiltInCommand
{
  // *- data members
public:
  ForegroundCommand(const char *cmd_line, JobsList *jobs);
  virtual ~ForegroundCommand() {}
  void execute() override;
};

/** Command number 7:
 * @brief
 */
class QuitCommand : public BuiltInCommand
{
  // *- data members
public:
  QuitCommand(const char *cmd_line, JobsList *jobs);
  virtual ~QuitCommand() {}
  void execute() override;
};

/** Command number 8:
 * @brief
 */
class KillCommand : public BuiltInCommand
{
  // *- data members
public:
  KillCommand(const char *cmd_line, JobsList *jobs);
  virtual ~KillCommand() {}
  void execute() override;
};

/*
 * External Commands
 */

/*
 * Special Commands
 */

/**
 * @brief
 */
class ChmodCommand : public BuiltInCommand
{
public:
  ChmodCommand(const char *cmd_line);
  virtual ~ChmodCommand() {}
  void execute() override;
};

/*
 * Small Shell class
 */

class SmallShell
{
private:
  // *- data members
  // * members
  std::string m_prompt; // DEFAULT_PROMPT
  JobsList m_background_jobs;

  // ? Private ctor cuz this class is a singleton
  SmallShell();

public:
  // * static
  static const std::string DEFAULT_PROMPT; // "smash"
  // * methods
  Command *CreateCommand(const char *cmd_line);
  SmallShell(SmallShell const &) = delete;     // disable copy ctor
  void operator=(SmallShell const &) = delete; // disable = operator
  static SmallShell &getInstance()             // make SmallShell singleton
  {
    static SmallShell instance; // Guaranteed to be destroyed.
    // Instantiated on first use.
    return instance;
  }
  ~SmallShell();
  void executeCommand(const char *cmd_line);
  // *- extra methods as needed
  const std::string &getPrompt() const;
  void setPrompt(const std::string &newPrompt);
};

#endif // SMASH_COMMAND_H_
