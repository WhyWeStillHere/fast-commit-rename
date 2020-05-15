#include <stdio.h>
#include <string.h>
#include <git2.h>

void HandleError(const int error) { 
    const git_error *e = git_error_last();
    printf("Error %d/%d: %s\n", error, e->klass, e->message);
    exit(error);
}

int main() {
    git_libgit2_init();
    git_repository *repo = NULL;
    int error = git_repository_open(&repo, "test_repo/");
    if (error < 0) {
        HandleError(error); 
    }

    const char new_message[] = "Redacted name";

    git_revwalk *walker;
    error = git_revwalk_new(&walker, repo);
    if (error < 0) {
        HandleError(error);
    }
    git_revwalk_sorting(walker, GIT_SORT_REVERSE);

    error = git_revwalk_push_range(walker, "HEAD~20..HEAD");
    if (error < 0) {
        HandleError(error);
    }

    git_oid oid;
    git_oid new_commit_oid;

    git_commit* commit;
    git_commit* fixed_commit = NULL;
    const git_oid* par_commit_oid;
    const git_oid* prev_commit_oid;
    size_t change_message = 0;

    while (!git_revwalk_next(&oid, walker)) {
        error = git_commit_lookup(&commit, repo, &oid);
        if (error < 0) {
            HandleError(error);
        }

        /// MANAGE INIT COMMIT!!!!!
        int parent_count = git_commit_parentcount(commit);
        printf("parents: %d\n", parent_count);
        git_commit* parents[parent_count];
        for (int i = 0; i < parent_count; ++i) {
            error = git_commit_parent(&parents[i], commit, i);
            if (error < 0) {
                HandleError(error);
            }
            par_commit_oid = git_commit_id(parents[i]);
            if ((fixed_commit != NULL) && 
                (git_oid_cmp(par_commit_oid, prev_commit_oid) == 0)) {
                parents[i] = fixed_commit;
            }
        }
        prev_commit_oid = git_commit_id(commit);

        git_tree* tree;
        git_commit_tree(&tree, commit);

        const char* message = git_commit_message(commit);

        if (change_message == 0) {
            change_message = 1;
            message = new_message;
        }

        error = git_commit_create(
            &new_commit_oid, 
            repo, 
            NULL,
            git_commit_author(commit),
            git_commit_committer(commit),
            git_commit_message_encoding(commit),
            message,
            tree,
            parent_count,
            (const git_commit**)parents // parent
            );
        if (error < 0) {
            HandleError(error);
        }
        
        git_commit_free(fixed_commit);
        git_commit_free(commit);

        error = git_commit_lookup(&fixed_commit, repo, &new_commit_oid);
    }

    git_revwalk_free(walker);

    git_reference* ref;
    error = git_repository_set_head_detached(repo, &new_commit_oid);
    if (error < 0) {
        HandleError(error);
    }
    error = git_branch_create(&ref, repo, "master", fixed_commit, 1);
    if (error < 0) {
        HandleError(error);
    }
    git_commit_free(fixed_commit);
    
    git_repository_free(repo);
    git_libgit2_shutdown();
}