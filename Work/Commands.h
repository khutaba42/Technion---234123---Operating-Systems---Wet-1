#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector> // for JobsList
#include <map>    // TODO piazza: for what?
#include <list>   // for ??

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)
#define COMMAND_MAX_PATH_LENGTH (80)

enum class CommandType
{
  BuiltIn,
  IO_Append,
  IO_Override,
  Pipe,
  Pipe_err,
  TimeOut,
  External
};
class Command
{
protected:
  enum RunType
  {
    ForeGround,
    BackGround
  };
  // *- data members
  // std::string m_cmd_line; //!! no need for now (imo)
  std::string m_name;
  std::vector<std::string> m_operands;
  CommandType m_type;
  RunType m_run_mode;

public:
  Command(){};
  Command(const std::string name, std::vector<std::string> m_operands, CommandType type, bool isBackground);
  virtual ~Command() = default;
  virtual void execute() = 0;
  // virtual void prepare();
  // virtual void cleanup();
  // *- extra methods as needed
  RunType getRunType() const
  {
    return m_run_mode;
  }
  const std::string &getCMDline() const;
  virtual unsigned int numOfArguments() const
  {
    return m_operands.size() - 1;
  }

  // ! We have to have the implementation here, because its templated.
  template <unsigned int N> // getArg<0> returns the name of the command, the others give the args in order
  const std::string &getArg() const
  {
    return m_operands[N];
  }
};

class BuiltInCommand : public Command
{
protected:
  std::vector<std::string> m_args;

public:
  BuiltInCommand(const std::string name, std::vector<std::string> operands, std::vector<std::string> args);
  virtual ~BuiltInCommand() {}
};

class ExternalCommand : public Command
{
  std::vector<std::string> m_args;

public:
  ExternalCommand(const std::string name, std::vector<std::string> operands, std::vector<std::string> args, bool isBackground);
  virtual ~ExternalCommand() {}
  void execute() override;
};

class PipeCommand : public Command
{
  // *- data members
  std::vector<std::string> m_args1;
  std::vector<std::string> m_args2; // used incase of pipe/io for the secound section of the command

public:
  PipeCommand(const std::string name, std::vector<std::string> operands, std::vector<std::string> args1, std::vector<std::string> args2, CommandType type);
  virtual ~PipeCommand() {}
  void execute() override;
};

class RedirectionCommand : public Command
{
  // *- data members
  std::vector<std::string> m_args1;
  std::vector<std::string> m_args2; // used incase of pipe/io for the secound section of the command
public:
  explicit RedirectionCommand(const std::string name, std::vector<std::string> operands, std::vector<std::string> args1, std::vector<std::string> args2, CommandType type);
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
  ChangePromptCommand(std::vector<std::string> operands, std::vector<std::string> args);
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
  ShowPidCommand(std::vector<std::string> operands, std::vector<std::string> args);
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
  GetCurrDirCommand(std::vector<std::string> operands, std::vector<std::string> args);
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
  ChangeDirCommand(std::vector<std::string> operands, std::vector<std::string> args);
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
    // TODO check if we should prettify the command
    std::string m_cmd_line;
    pid_t m_command_pid;
    unsigned long m_id;
    bool m_isStopped;
    JobEntry(const std::string &cmdLine, unsigned long id, bool isStopped) : m_cmd_line(cmdLine), m_id(id), m_isStopped(isStopped){};
  };
  // *- data members
  std::list<JobEntry> m_jobsList;

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
  bool isEmpty() const;
};

/** Command number 5:
 * @brief
 */
class JobsCommand : public BuiltInCommand
{
  // *- data members
  JobsList m_jobs;

public:
  JobsCommand(std::vector<std::string> operands, std::vector<std::string> args, const JobsList &jobs);
  virtual ~JobsCommand() {}
  void execute() override;
};

/** Command number 6:
 * @brief
 */
class ForegroundCommand : public BuiltInCommand
{
  // *- data members
  JobsList m_jobs;
  int m_jobID;

public:
  ForegroundCommand(std::vector<std::string> operands, std::vector<std::string> args, JobsList &jobs);
  virtual ~ForegroundCommand() {}
  void execute() override;
};

/** Command number 7:
 * @brief
 */
class QuitCommand : public BuiltInCommand
{
  // *- data members
  JobsList m_jobs;

public:
  QuitCommand(std::vector<std::string> operands, std::vector<std::string> args, JobsList &jobs);
  virtual ~QuitCommand() {}
  void execute() override;
};

/** Command number 8:
 * @brief
 */
class KillCommand : public BuiltInCommand
{
  // *- data members
  JobsList m_jobs;
  int m_sigNum;
  int m_jobID;

public:
  KillCommand(std::vector<std::string> operands, std::vector<std::string> args, JobsList &jobs);
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
  ChmodCommand(std::vector<std::string> operands, std::vector<std::string> args);
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
