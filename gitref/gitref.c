#include <git2.h>
#include <string.h>
#include <stdio.h>

static git_repository *repo;
static git_reference *headref;
static const git_oid *headoid;


static const char *
trim(const char *name) {
  if(strstr(name, "refs/remotes/") == name)
    name += 13;
  else if(strstr(name, "refs/heads/") == name)
    name += 11;

  return name;
}


static int
ref_callback(const char *name, void __attribute__((unused)) *payload) {
  git_reference *ref;
  int err = git_reference_lookup(&ref, repo, name);
  if(err)
    return err;

  const git_oid *oid = git_reference_target(ref);
  if(oid && git_oid_equal(headoid, oid) && strcmp(name, git_reference_name(headref))) {
    printf(" %s", trim(name));
  }
  
  git_reference_free(ref);
    
  return 0;
}


static int
tag_callback(const char *name, git_oid *tag_id,
	     void __attribute__((unused)) *payload) {
  git_tag *tag;
  int err = git_tag_lookup(&tag, repo, tag_id);
  if(err == GIT_ENOTFOUND) { // lightweight tag
    if(git_oid_equal(headoid, tag_id)) {
      char *ptr = strrchr(name, '/');
      if(ptr)
	name = ptr + 1;

      printf(" %s", name);
    }
    return 0;
  }
  if(err) {
    return err;
  }

  const git_oid *oid = git_tag_target_id(tag);
  if(git_oid_equal(headoid, oid)) {
    char *ptr = strrchr(name, '/');
    if(ptr)
      name = ptr + 1;

    printf(" %s", name);
  }
  
  git_tag_free(tag);

  return 0;
}

  
int
main() {
  git_libgit2_init();

  git_buf buf;
  memset(&buf, 0, sizeof(buf));
	 
  int err = git_repository_discover(&buf, ".", 1, NULL);
  if(err) {
    if(err == GIT_ENOTFOUND)
      err = 0;
    goto out;
  }
  
  err = git_repository_open(&repo, buf.ptr);
  if(err)
    goto out_discover;

  err = git_repository_head(&headref, repo); 
  if(err)
    goto out_open;

  headoid = git_reference_target(headref);

  if(git_repository_head_detached(repo)) {
    printf("[%.8s", git_oid_tostr_s(headoid));
  } else {
    printf("[%s", trim(git_reference_name(headref)));
  }
  
  err = git_reference_foreach_glob(repo, "refs/remotes/*", ref_callback, NULL);
  if(err)
    goto out_reference;

  err = git_reference_foreach_glob(repo, "refs/heads/*", ref_callback, NULL);
  if(err)
    goto out_reference;

  err = git_tag_foreach(repo, tag_callback, NULL);
  if(err)
    goto out_reference;

  fputs("] \n", stdout);

 out_reference:
  git_reference_free(headref);
  
 out_open:
  git_repository_free(repo);

 out_discover:
  git_buf_free(&buf);
  
 out:
  git_libgit2_shutdown();

  return err;
}
