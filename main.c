#include <stdio.h>
#include <string.h>
#include <git2.h>

void HandleError(const int error) {
    const git_error *e = git_error_last();
    printf("Error %d/%d: %s\n", error, e->klass, e->message);
    exit(error);
}

int main(int argc, char* argv[]) {
    if (argc <= 2) {
        fprintf(stderr,"To few argc");
        exit(1);
    }

    char* commit_ref = argv[1];
    const char* new_message = argv[2];
    const char repo_name[] = "test_repo/";

    git_libgit2_init();
    git_repository *repo = NULL;
    int error = git_repository_open(&repo, repo_name);
    if (error < 0) {
        HandleError(error);
    }

    git_revwalk *walker;
    error = git_revwalk_new(&walker, repo);
    if (error < 0) {
        HandleError(error);
    }

    git_revwalk_sorting(walker, GIT_SORT_REVERSE);

    git_object* commit_object;
    error = git_revparse_single(&commit_object, repo, commit_ref);
    git_commit* commit_to_change = (git_commit*)commit_object;

    git_oid current_commit_oid = *git_commit_id(commit_to_change);

    git_revwalk_hide(walker, &current_commit_oid);
    git_revwalk_push_head(walker);

    git_commit_free(commit_to_change);

    git_oid new_commit_oid;

    git_commit* current_commit;
    git_commit* previous_changed_commit = NULL;
    git_oid previous_commit_oid;
    size_t is_message_changed = 0;

    do {
        error = git_commit_lookup(&current_commit, repo, &current_commit_oid);
        if (error < 0) {
            HandleError(error);
        }

        size_t parent_count = git_commit_parentcount(current_commit);
        git_commit* parents[parent_count];

        for (int i = 0; i < parent_count; ++i) {
            error = git_commit_parent(&parents[i], current_commit, i);
            if (error < 0) {
                HandleError(error);
            }
            const git_oid* parent_commit_oid = git_commit_id(parents[i]);
            
            if ((previous_changed_commit != NULL) &&
                (git_oid_cmp(parent_commit_oid, &previous_commit_oid) == 0)) {
                git_commit_free(parents[i]);
                parents[i] = previous_changed_commit;
            }
        }
        previous_commit_oid = current_commit_oid;

        git_tree* tree;
        git_commit_tree(&tree, current_commit);

        const char* message = git_commit_message(current_commit);

        if (is_message_changed == 0) {
            is_message_changed = 1;
            message = new_message;
        }

        error = git_commit_create(
                &new_commit_oid,
                repo,
                NULL,
                git_commit_author(current_commit),
                git_commit_committer(current_commit),
                git_commit_message_encoding(current_commit),
                message,
                tree,
                parent_count,
                (const git_commit**)parents
        );
        if (error < 0) {
            HandleError(error);
        }

        git_tree_free(tree);

        //Fixed commit free there
        for (int i = 0; i < parent_count; ++i) {
            git_commit_free(parents[i]);
        }

        git_commit_free(current_commit);

        error = git_commit_lookup(&previous_changed_commit, repo, &new_commit_oid);

    } while (!git_revwalk_next(&current_commit_oid, walker));

    git_revwalk_free(walker);

    git_reference* ref;

    error = git_repository_set_head_detached(repo, &new_commit_oid);
    if (error < 0) {
        HandleError(error);
    }


    // Change master!!
    error = git_branch_create(&ref, repo, "master", previous_changed_commit, 1);
    if (error < 0) {
        HandleError(error);
    }

    git_reference_free(ref);
    git_commit_free(previous_changed_commit);
    git_repository_free(repo);
    git_libgit2_shutdown();
}
