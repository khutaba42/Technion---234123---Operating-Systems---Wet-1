#ifndef SMASH_COMMAND_H_
#define SMASH_COMMAND_H_

#include <vector>
#include <list>
#include <string>

#define COMMAND_ARGS_MAX_LENGTH (200)
#define COMMAND_MAX_ARGS (20)

/**
 * All commands has the following atributes
 *    the command_line
 *    are background or foreground (this can be ignored during the command execution)
 * Not all commands has a name (pipe for example) so we wont have anything else here
 */
class Command
{
  /* types */
  enum class GroundType
  {
    Foreground,
    Background
  };
  /* variables */
  GroundType m_ground_type; // should come before the command line
  std::string m_cmd_line;   // command line
  bool m_valid;

public:
  /* methods */
  Command(const char *cmd_line);
  virtual ~Command();
  virtual void execute() = 0;
  // virtual void prepare(); // ? what are these
  // virtual void cleanup(); // ? what are these
  const std::string &getCMDLine() const { return m_cmd_line; }
  bool isBackground() const { return m_ground_type == GroundType::Background; }
  std::string m_remove_background_sign(const char *cmd_line) const;

  void invalidate_command() { m_valid = false; }
  bool is_valid() const { return m_valid; }
};

/*
 * External Commands
 */

/* *
 * I cant see any more members to add for the external commands
 * other than if it is simple or not
 *
 */
class ExternalCommand : public Command
{
  /* types */
  enum class Complexity
  {
    Simple,
    Complex
  };
  Complexity m_complexity;

  /* methods */
  Complexity _get_complexity_type(const char *cmd_line);

public:
  ExternalCommand(const char *cmd_line);
  virtual ~ExternalCommand();
  void execute() override;
};

/*
 * Special Commands
 */

/* *
 * The pipe command contains 2 commands and the type of piping (| or |&)
 * If you see the character "|" or "|&" in the command, then its a pipe :-)
 */
class PipeCommand : public Command
{
public:
  /* types */
  enum class PipeType
  {
    Standard,
    Error
  };

  /* methods */
  PipeCommand(const char *cmd_line);
  virtual ~PipeCommand();
  void execute() override;

private:
  /* variables */
  PipeType m_pipe_type;
  Command *m_cmd_1;
  Command *m_cmd_2;

  PipeCommand::PipeType _get_pipe_type(const char *cmd_line);
  Command *_get_cmd_1(const char *cmd_line);
  Command *_get_cmd_2(const char *cmd_line);
};

/* *
 * The RedirectionCommand command contains 1 command first and
 * a file second separated by a `>` or a `>>` for overriding
 * and appending respectively.
 * If you see the character ">" or ">>" in the command, then its a RedirectionCommand
 */
class RedirectionCommand : public Command
{
  /* types */
  enum class RedirectionType
  {
    Override,
    Append
  };
  /* variables */
  RedirectionType m_redirection_type;
  Command *m_command;
  std::string m_file_path; // the path can be absolute or relative

  RedirectionType get_redirection_type(const char *cmd_line);

public:
  /* methods */
  explicit RedirectionCommand(const char *cmd_line);
  virtual ~RedirectionCommand();
  void execute() override;
  // void prepare() override; // ? what are these
  // void cleanup() override; // ? what are these
};

/*
 * Built In Commands
 */

/* *
 * All built in commands (+chmod) has a name and arguments
 * They all don't run in the background (ignore & at the end)
 * meaning they all will run in the foreground (no forking)
 */
class BuiltInCommand : public Command
{
public:
  /* methods */
  BuiltInCommand(const char *cmd_line);
  virtual ~BuiltInCommand();
  virtual void execute() = 0;

  unsigned int numOfArgs() const { return m_args.size(); }
  const std::string &getName() const { return m_name; }
  const std::vector<std::string> &getArgs() const { return m_args; }

private:
  /* variables */
  std::string m_name;
  std::vector<std::string> m_args;
  /* methods */
  std::string m_parse_name(const char *cmd_line) const;
  std::vector<std::string> m_parse_args(const char *cmd_line) const;
};

/** Command number 1:
 * @brief `chprompt` command will allow the user to change the prompt displayed by the smash while waiting for the next command.
 *    If no parameters were sent, then the prompt shall be reset to smash. If more than one parameter was sent, then the rest shall be ignored.
 *    Note that this command will not change the prompt in error messages that we will see later.
 */
class ChangePromptCommand : public BuiltInCommand
{
public:
  ChangePromptCommand(const char *cmd_line);
  virtual ~ChangePromptCommand();
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
  virtual ~ShowPidCommand();
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
  virtual ~GetCurrDirCommand();
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
  /* static variables */
  static std::list<std::string> CD_PATH_HISTORY;

  /* methods */
  std::string get_parent_directory(const std::string &path) const;

public:
  ChangeDirCommand(const char *cmd_line);
  virtual ~ChangeDirCommand();
  void execute() override;
};

/* forward declare JobsList */
class JobsList;

/** Command number 5:
 * @brief
 */
class JobsCommand : public BuiltInCommand
{
public:
  JobsCommand(const char *cmd_line);
  virtual ~JobsCommand();
  void execute() override;
};

/** Command number 6:
 * @brief
 */
class ForegroundCommand : public BuiltInCommand
{
  int m_id;
public:
  ForegroundCommand(const char *cmd_line);
  virtual ~ForegroundCommand();
  void execute() override;
};

/** Command number 7:
 * @brief
 */
class QuitCommand : public BuiltInCommand
{
public:
  QuitCommand(const char *cmd_line);
  virtual ~QuitCommand();
  void execute() override;
};

/** Command number 8:
 * @brief
 */
class KillCommand : public BuiltInCommand
{
  /* variables */
  int m_signal_number;
  unsigned int m_job_id;

public:
  KillCommand(const char *cmd_line);
  virtual ~KillCommand();
  void execute() override;
};

/**
 * @brief
 */
class ChmodCommand : public BuiltInCommand
{
public:
  ChmodCommand(const char *cmd_line);
  virtual ~ChmodCommand();
  void execute() override;
};

/* *
 * The JobsList class
 */

class JobsList
{
public:
  class JobEntry
  {
  public:
    /* methods */
    JobEntry(Command *command, pid_t job_pid, unsigned int job_id);
    Command *getCommand();
    pid_t getJobPid();
    unsigned int getJobID();

  private:
    /* variables */
    Command *m_command;
    pid_t m_job_pid;       // since the job is run in the background we must have used fork()
    unsigned int m_job_id; // the job id in the list
  };

  /* methods */
  unsigned int size() const;
  std::list<JobEntry> &getList();

  JobsList();
  ~JobsList();
  void addJob(Command *cmd, pid_t pid);
  void printJobsList();
  void killAllJobs();
  void removeFinishedJobs();
  JobEntry *getJobById(int jobId);
  void removeJobById(int jobId);
  JobEntry *getLastJob(int *lastJobId);
  JobEntry *getLastStoppedJob(int *jobId);

private:
  /* variables */
  std::list<JobEntry> m_jobs;
};

/* *
 * The Small Shell class
 */

class SmallShell
{
public:
  /* static variables */
  static const std::string DEFAULT_PROMPT; // originally set to "smash" (in .cpp)

  /* methods */
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

  JobsList &getJobsList();
  const std::string &getPrompt() const;
  void setPrompt(const std::string &newPrompt);

private:
  /* variables */
  std::string m_prompt; // originally set to DEFAULT_PROMPT
  JobsList m_background_jobs;

  int m_currForegroundPID;

  /* methods */
  SmallShell(); // private c'tor

  Command *CreateCommand_aux(const char *cmd_line);
};

#endif // SMASH_COMMAND_H_
