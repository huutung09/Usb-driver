#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <openssl/sha.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>
#define MAX_STUDENTS 100
#define SHA1_HASH_SIZE 20

typedef struct {
    int id;
    char name[50];
    int age;
    char class[20];
    float point;
} Student;

Student students[MAX_STUDENTS];
int numStudents = 0;
int fd;
        
void addStudent() {
    if (numStudents >= MAX_STUDENTS) {
        printf("Maximum number of students reached.\n");
        return;
    }

    Student newStudent;

    printf("Enter student ID: ");
    scanf("%d", &newStudent.id);

    printf("Enter student name: ");
    scanf(" %[^\n]", newStudent.name);

    printf("Enter student age: ");
    scanf("%d", &newStudent.age);

    printf("Enter student class: ");
    scanf(" %[^\n]", newStudent.class);

    printf("Enter student point: ");
    scanf("%f", &newStudent.point);

    students[numStudents] = newStudent;
    numStudents++;

    printf("Student added successfully.\n");
}

void writeFile() {
    char write_buf[1024] = "";
    int isAppend = 0;
    if (numStudents == 0) {
        printf("No students in the list.\n");
        return;
    }
	
    for (int i = 0; i < numStudents; i++) {
    	write_buf[1024] = "";
        snprintf(write_buf, 1024, "%d/%d/%s/%d/%s/%.2f\0",isAppend, students[i].id, students[i].name, students[i].age, students[i].class, students[i].point);
        printf("concat string: %s\n", write_buf);

    	write(fd, write_buf, strlen(write_buf));
    	isAppend = 1;
    }


    printf("Student list exported to file 'student_list.txt'.\n");
}

		
void loadStudentsFromFile() {	
    char read_buf[1024];
    ssize_t bytes_read = read(fd, read_buf, sizeof(read_buf));
if (bytes_read < 0) {
    perror("Failed to read from the device file");
    close(fd);
    return;
}
char *line = read_buf;

//memset(students, 0, numStudents);
//numStudents = 0;
while (*line != '\0') {
    int id, age;
    char name[50], class[20], point[10];
    int result = sscanf(line, "%d\t%49s\t%d\t%19s\t%9s\n", &id, name, &age, class, point);
    if (result == 5) {
        // Process the student data here

        students[numStudents].id = id;
        strncpy(students[numStudents].name, name, 50);
        students[numStudents].age = age;
        strncpy(students[numStudents].class, class, 20);
	students[numStudents].point = atof(point);
	numStudents++;
    }
    // Move to the next line
    line = strchr(line, '\n');
    if (!line)
        break;
    line++; // Skip the newline character
}

    for (int i = 0; i < numStudents; i++) {
        printf("%d\t%s\t%d\t%s\t%.2f\n", students[i].id, students[i].name, students[i].age, students[i].class, students[i].point);
    }
}

int verifyPassword(const char *inputPassword, const unsigned char *storedHash) {
    unsigned char hash[SHA1_HASH_SIZE];
    SHA1((const unsigned char *)inputPassword, strlen(inputPassword), hash);

    for (int i = 0; i < SHA1_HASH_SIZE; i++) {
        if (hash[i] != storedHash[i]) {
            return 0;  // Password verification failed
        }
    }

    return 1;  // Password verification successful
}

int login() {
    char username[50];
    char password[50];
    unsigned char storedHash[SHA1_HASH_SIZE] = { 0 };

    // Convert hexadecimal string to byte array
    const char *hexString = "6367C48DD193D56EA7B0BAAD25B19455E529F5EE";
    for (int i = 0; i < SHA1_HASH_SIZE * 2; i += 2) {
        sscanf(hexString + i, "%2hhx", &storedHash[i / 2]);
    }

    printf("Username: ");
    scanf("%s", username);

    printf("Password: ");
    scanf("%s", password);

    if (verifyPassword(password, storedHash)) {
        printf("Login successful.\n");
        return 1;
    } else {
        printf("Invalid username or password.\n");
        return 0;
    }
}

void updateStudent() {
    if (numStudents == 0) {
        printf("No students in the list.\n");
        return;
    }

    int studentId;
    int found = 0;

    printf("Enter student ID to update: ");
    scanf("%d", &studentId);

    for (int i = 0; i < numStudents; i++) {
        if (students[i].id == studentId) {
            found = 1;

            printf("Enter updated student name: ");
            scanf(" %[^\n]", students[i].name);

            printf("Enter updated student age: ");
            scanf("%d", &students[i].age);

            printf("Enter updated student class: ");
            scanf(" %[^\n]", students[i].class);

            printf("Enter updated student point: ");
            scanf("%f", &students[i].point);

            printf("Student information updated successfully.\n");
            break;
        }
    }

    if (!found) {
        printf("Student with ID %d not found.\n", studentId);
    }
}

void deleteStudent() {
    if (numStudents == 0) {
        printf("No students in the list.\n");
        return;
    }

    int studentId;
    int found = 0;

    printf("Enter student ID to delete: ");
    scanf("%d", &studentId);

    for (int i = 0; i < numStudents; i++) {
        if (students[i].id == studentId) {
            found = 1;

            // Shift elements to overwrite the deleted student
            for (int j = i; j < numStudents - 1; j++) {
                students[j] = students[j + 1];
            }

            numStudents--;

            printf("Student with ID %d deleted successfully.\n", studentId);
            break;
        }
    }

    if (!found) {
        printf("Student with ID %d not found.\n", studentId);
    }
}

int main() {

    fd = open("/dev/huutung0", O_RDWR);//mo de doc va ghi
    if(fd < 0) {
         printf("Cannot open device file...\n");
         return 0;
    }
    
    if (!login()) {
        printf("Exiting program.\n");
        close(fd);
        return 0;
    }

    int choice;

 do {	
        printf("\nStudent List Management\n");	
        printf("1. Add Student\n");	
        printf("2. Export to File\n");	
        printf("3. Display Student\n");	
        printf("4. Update Student\n");	
        printf("5. Delete Student\n");	
        printf("6. Quit\n");	
        printf("Enter your choice: ");	
        scanf("%d", &choice);	
        switch (choice) {	
            case 1:	
                addStudent();	
                break;	
            case 2:	
                writeFile();	
                break;	
            case 3:	
                loadStudentsFromFile();	
                break;	
            case 4:	
                updateStudent();
                //writeFile();		
                break;	
            case 5:	
                deleteStudent();	
                //writeFile();	
                break;	
            case 6:	
                printf("Exiting program.\n");	
                break;	
            default:	
                printf("Invalid choice. Please try again.\n");	
                break;	
        }	
    } while (choice != 6);

    return 0;
}

