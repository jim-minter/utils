#include <sys/types.h>
#include <sys/stat.h>
#include <string.h>
#include <time.h>
#include <unistd.h>

static char *
itoa(int val) {
  static char buf[32];
  int i;
	
  for(i = 30; val && i; i--, val /= 10)
    buf[i] = '0' + val % 10;
	
  return buf + i + 1;
}

static inline void
print(const char *s) {
  write(1, s, strlen(s));
}

int
main() {
  struct stat st;
  if(stat("/home/jminter/.last_backup", &st) == -1) {
    print("\n\e[31;1mBackup required.\e[0m\n\n");
    return 1;
  }
  
  int ago = (time(NULL) - st.st_mtime) / 86400;

  if(ago >= 28) {
    print("\n\e[31;1mLast full backup was ");
    print(itoa(ago));
    print(" days ago.\e[0m\n\n");
    return 1;
  }

  return 0;
}
