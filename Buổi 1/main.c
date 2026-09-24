#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define MAX_LEN      64
#define USER_FILE    "user.txt"
#define MAX_ATTEMPTS 3

typedef struct User {
    char        username[MAX_LEN];
    char        password[MAX_LEN];
    int         status; // 0=active, 1=blocked
    float       score;
    struct User *next;
} User;

User *head = NULL;

static User *createNode(const char *u, const char *p, int s, float sc)
{
    User *node = malloc(sizeof(User));
    if (!node) return NULL;
    strncpy(node->username, u, MAX_LEN - 1); node->username[MAX_LEN - 1] = '\0';
    strncpy(node->password, p, MAX_LEN - 1); node->password[MAX_LEN - 1] = '\0';
    node->status = s; node->score = sc; node->next = NULL;
    return node;
}

static void appendNode(User *node)
{
    if (!head) { head = node; return; }
    User *cur = head;
    while (cur->next) cur = cur->next;
    cur->next = node;
}

static User *findUser(const char *username)
{
    for (User *cur = head; cur; cur = cur->next)
        if (strcmp(cur->username, username) == 0) return cur;
    return NULL;
}

// Đọc dữ liệu từ file vào linked list
void loadUsers(void)
{
    FILE *f = fopen(USER_FILE, "r");
    if (!f) return;
    char u[MAX_LEN], p[MAX_LEN]; int s; float sc;
    while (fscanf(f, "%63[^:]:%63[^:]:%d:%f\n", u, p, &s, &sc) == 4)
        appendNode(createNode(u, p, s, sc));
    fclose(f);
}

// Ghi toàn bộ danh sách ra file (overwrite)
void saveUsers(void)
{
    FILE *f = fopen(USER_FILE, "w");
    if (!f) return;
    for (User *cur = head; cur; cur = cur->next)
        fprintf(f, "%s:%s:%d:%.2f\n", cur->username, cur->password, cur->status, cur->score);
    fclose(f);
}

void freeList(void)
{
    User *cur = head;
    while (cur) { User *next = cur->next; free(cur); cur = next; }
    head = NULL;
}

void printMenu(void)
{
    printf("\nUSER MANAGEMENT PROGRAM\n");
    printf("-----------------------------------\n");
    printf("1. Register\n2. Sign in\n3. Search\n4. Sort\n");
    printf("Your choice (1-4, other to quit): ");
    fflush(stdout);
}

// 1. Register
void registerUser(void)
{
    char u[MAX_LEN], p[MAX_LEN]; float sc;

    printf("Username: "); scanf("%63s", u);
    if (findUser(u)) { printf("Account existed\n"); return; }

    printf("Password: "); scanf("%63s", p);
    printf("Score: ");    scanf("%f",   &sc);

    appendNode(createNode(u, p, 0, sc));

    FILE *f = fopen(USER_FILE, "a");
    if (f) { fprintf(f, "%s:%s:%d:%.2f\n", u, p, 0, sc); fclose(f); }

    printf("Successful registration\n");
}

// 2. Sign in
void signIn(void)
{
    char u[MAX_LEN], p[MAX_LEN];
    int attempts = 0;

    printf("Username: "); scanf("%63s", u);

    User *user = findUser(u);
    if (!user)             { printf("Cannot find account\n");   return; }
    if (user->status == 1) { printf("Account is blocked\n");    return; }

    while (attempts < MAX_ATTEMPTS) {
        printf("Password: "); scanf("%63s", p);
        if (strcmp(p, user->password) == 0) {
            printf("Hello %s\n", u);
            return;
        }
        attempts++;
        if (attempts < MAX_ATTEMPTS)
            printf("Password is incorrect\n");
        else {
            // Nhập sai 3 lần thì khóa tài khoản
            user->status = 1;
            saveUsers();
            printf("Password is incorrect. Account is blocked\n");
        }
    }
}

// 3. Search
void searchUser(void)
{
    char u[MAX_LEN];
    printf("Username: "); scanf("%63s", u);

    User *user = findUser(u);
    if (!user) { printf("Cannot find account\n"); return; }

    printf("Account is %s\n", user->status == 0 ? "active" : "blocked");
}

// 4. Sort & Search
void sortAndSearch(void)
{
    int n = 0;
    for (User *cur = head; cur; cur = cur->next) n++;
    if (n == 0) { printf("No accounts.\n"); return; }

    // Copy sang mảng để dùng random access
    User **arr = malloc(n * sizeof(User *));
    User *cur = head;
    for (int i = 0; i < n; i++, cur = cur->next) arr[i] = cur;

    // Bubble sort giảm dần theo score (swap dữ liệu, giữ nguyên con trỏ next)
    for (int i = 0; i < n - 1; i++) {
        for (int j = 0; j < n - 1 - i; j++) {
            if (arr[j]->score < arr[j + 1]->score) {
                char tu[MAX_LEN], tp[MAX_LEN]; int ts; float tsc;
                strncpy(tu, arr[j]->username, MAX_LEN);
                strncpy(tp, arr[j]->password,  MAX_LEN);
                ts = arr[j]->status; tsc = arr[j]->score;

                strncpy(arr[j]->username, arr[j+1]->username, MAX_LEN);
                strncpy(arr[j]->password, arr[j+1]->password,  MAX_LEN);
                arr[j]->status = arr[j+1]->status; arr[j]->score = arr[j+1]->score;

                strncpy(arr[j+1]->username, tu, MAX_LEN);
                strncpy(arr[j+1]->password, tp, MAX_LEN);
                arr[j+1]->status = ts; arr[j+1]->score = tsc;
            }
        }
    }

    printf("Sắp xếp:\n");
    for (int i = 0; i < n; i++)
        printf("%s:%g\n", arr[i]->username, arr[i]->score);

    // Binary search: tìm vị trí cuối cùng có score > 5 trong mảng giảm dần
    int lo = 0, hi = n - 1, last = -1;
    while (lo <= hi) {
        int mid = (lo + hi) / 2;
        if (arr[mid]->score > 5.0f) { last = mid; lo = mid + 1; }
        else hi = mid - 1;
    }

    printf("\nTìm kiếm:\n");
    if (last == -1) {
        printf("No account with score > 5.\n");
    } else {
        for (int i = 0; i <= last; i++)
            printf("%s\t%g\n", arr[i]->username, arr[i]->score);
    }

    free(arr);
}

int main(void)
{
    loadUsers();

    int choice;
    do {
        printMenu();
        if (scanf("%d", &choice) != 1) break;
        switch (choice) {
            case 1: registerUser();  break;
            case 2: signIn();        break;
            case 3: searchUser();    break;
            case 4: sortAndSearch(); break;
            default: choice = 0;    break;
        }
    } while (choice >= 1 && choice <= 4);

    freeList();
    return 0;
}
