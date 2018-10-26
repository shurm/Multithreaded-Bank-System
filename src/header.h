typedef struct BankAccount Account;
struct BankAccount
{
	char name[101];
	float balance;
	int inSession;
	
};

typedef struct SComposite SPackage;
struct SComposite
{
	int length;
	Account* accounts;
	
};
typedef struct CComposite CPackage;
struct CComposite
{
	Account* account;
	int* active;
	int id;
	int length;
	int index;
};
typedef struct Strings TwoStrings;
struct Strings
{
	char* command;
	char* name;
};

typedef void* (*Function)( void * );
int createThread(pthread_t* id,Function f,void *arg);
int change(int* ptr, int length,int type);
void extractInfo(char* fullLine, char* command, char* name);
void clearAccounts(Account * accounts, int max);

