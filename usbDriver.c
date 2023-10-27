#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/usb.h>
#include <linux/fs.h>
#include <linux/file.h>
#define USB_VENDOR_ID       ( 0x0483 )      //USB device's vendor ID
#define USB_PRODUCT_ID      ( 0x3748 )      //USB device's product ID
#define MAX_STUDENTS 100
#define MAX_BUFFER_SIZE 256
#define DATA_DIR "/home/tung/Documents/usbDriverProject/student_list.txt"
static struct usb_device *device;
static struct usb_class_driver class;
char kernel_buffer[1024];


typedef struct {
    int id;
    char name[50];
    int age;
    char class[20];
    char point[10];
} Student;

Student students[MAX_STUDENTS];
int numStudents = 0;



//Hàm biến đổi ký tự thường thành ký tự hoa
char chuHoa(char charx){
    if(charx >= 'a' && charx <='z') charx = charx-32;
    return charx;
}
 
 
//Hàm chuẩn hóa xâu
void chuanHoa(char a[])
{
    int n = strlen(a); //Lấy độ dài xâu
 
//Xóa khoảng trắng đầu xâu
    for(int i=0;i<n;i++){
        if(a[i]==' '){
            for(int j=i;j<n-1;j++){
                a[j] = a[j+1];
            }
            a[n-1]=NULL;
            i--;
            n--;
        }
        else break;
    }
     
 
//Xóa khoảng trắng cuối xâu
    for(int i=n-1;i>=0;i--){
        if(a[i]==' '){
            a[i]=NULL;
            n--;
        }
        else break;
    }
     
    //Xóa khoảng trắng không hợp lệ(khoảng trắng bị thừa)
    for(int i=1;i<n-1;i++)
    {
        if(a[i]==a[i+1] && a[i] == ' '){
            for(int j=i;j<n-1;j++){
                a[j] = a[j+1];
            }
            a[n-1]=NULL;
            i--;
            n--;
        }
    }
     
//In hoa ký tự đầu tiên
    a[0] =  chuHoa(a[0]);  
}

static void read_from_file(const char *filename)
{
    struct file *file;
    loff_t offset;
    char *buf;
    ssize_t bytes_read;

    // Open the file in read mode
    file = filp_open(filename, O_RDONLY, 0);
    if (IS_ERR(file)) {
        printk(KERN_ERR "Failed to open file: %s\n", filename);
        return;
    }

    // Allocate memory for buffer
    buf = kmalloc(MAX_BUFFER_SIZE, GFP_KERNEL);
    if (!buf) {
        printk(KERN_ERR "Failed to allocate memory.\n");
        filp_close(file, NULL);
        return;
    }

    // Set the file position to the beginning
    offset = 0;
    vfs_llseek(file, offset, SEEK_SET);

    // Read data from the file
    bytes_read = kernel_read(file, buf, MAX_BUFFER_SIZE - 1, &offset);

    if (bytes_read < 0) {
        printk(KERN_ERR "Failed to read from file: %s\n", filename);
        kfree(buf);
        filp_close(file, NULL);
        return;
    }

    // Null-terminate the buffer
    buf[bytes_read] = '\0';

	printk("buffer: %s", buf);
    // Process the read data
    // Assuming the file contains lines in the format: "%d\t%s\t%d\t%s\t%s\n"
    char *line = buf;
    while (*line != '\0') {
        int id, age;
        char name[50], class[20], point[10];
        int result = sscanf(line, "%d\t%49s\t%d\t%19s\t%9s\n", &id, name, &age, class, point);
        if (result == 5) {
            
        students[numStudents].id = id;
        strncpy(students[numStudents].name, name, 50);
        students[numStudents].age = age;
        strncpy(students[numStudents].class, class, 20);

        strncpy(students[numStudents].point, point, 10);
                        numStudents++;

        }
        // Move to the next line
        line = strchr(line, '\n');
        if (!line)
            break;
        line++; // Skip the newline character
    }

    // Cleanup and close the file
    kfree(buf);
    filp_close(file, NULL);
        printk("Loaded Student List:\n");
    printk("ID\tName\tAge\tClass\tPoint\n");
    for (int i = 0; i < numStudents; i++) {
        printk("%d\t%s\t%d\t%s\t%s\n", students[i].id, students[i].name, students[i].age, students[i].class, students[i].point);
    }
}



static void append_to_file(const char *filename, const char *format, ...)
{
    struct file *file;
    loff_t pos;
    char buf[256];  // Buffer to store the formatted data
    va_list args;

    // Open the file in append mode
    file = filp_open(filename, O_WRONLY | O_APPEND | O_CREAT, 0644);
    if (IS_ERR(file)) {
        printk(KERN_ERR "Failed to open file: %s\n", filename);
        return;
    }

    // Set the file position to the end of the file
    pos = file->f_pos;
    vfs_llseek(file, pos, SEEK_END);

    // Format the data using vsnprintf
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);

    // Write the formatted data to the file
    kernel_write(file, buf, strlen(buf), &pos);

    // Close the file
    filp_close(file, NULL);
}

static void write_to_file(const char *filename, const char *format, ...)
{
    struct file *file;
    loff_t pos;
    char buf[256];  // Buffer to store the formatted data
    va_list args;

    // Open the file in write mode (truncating it)
    file = filp_open(filename, O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (IS_ERR(file)) {
        printk(KERN_ERR "Failed to open file: %s\n", filename);
        return;
    }

    // Set the file position to the beginning of the file
    pos = 0;
    vfs_llseek(file, pos, SEEK_SET);

    // Format the data using vsnprintf
    va_start(args, format);
    vsnprintf(buf, sizeof(buf), format, args);
    va_end(args);

    // Write the formatted data to the file
    kernel_write(file, buf, strlen(buf), &pos);

    // Close the file
    filp_close(file, NULL);
}


static int usb_open(struct inode *i, struct file *f)
{
    read_from_file(DATA_DIR);
    printk("usb open from driver");
    return 0;
}
static int usb_close(struct inode *i, struct file *f)
{
    printk("usb close from driver");
    return 0;
}
static ssize_t usb_read(struct file *f, char __user *buf, size_t cnt, loff_t *off)
{
    printk("usb read from driver");
    int i;
    char temp_buf[MAX_BUFFER_SIZE];
    char *user_buf = buf;
    size_t total_size = 0;

    // Prepare the data to be transferred
    for (i = 0; i < numStudents; i++) {
        snprintf(temp_buf, sizeof(temp_buf), "%d\t%s\t%d\t%s\t%s\n",
                 students[i].id, students[i].name, students[i].age,
                 students[i].class, students[i].point);

        size_t temp_size = strlen(temp_buf);

        // Check if there is enough space in the user buffer
        if (total_size + temp_size > cnt)
            break;

        // Copy the data to the user buffer
        if (copy_to_user(user_buf, temp_buf, temp_size) != 0)
            return -EFAULT;

        user_buf += temp_size;
        total_size += temp_size;
    }

    // Return the total number of bytes read
    return total_size;
}
static ssize_t usb_write(struct file *f, const char __user *buf, size_t cnt,
                                    loff_t *off)
{
    Student data;
    int isAppend = 0;
    copy_from_user(kernel_buffer, buf, cnt);

    sscanf(kernel_buffer, "%d/%d/%19[^/]/%d/%19[^/]/%19[^/]", &isAppend,
           &data.id, data.name, &data.age, data.class, data.point);
    chuanHoa(data.name);
    chuanHoa(data.class);
    chuanHoa(data.point);
    if (isAppend == 0) {
    	 write_to_file(DATA_DIR, "%d\t%s\t%d\t%s\t%s\n", data.id, data.name, data.age, data.class, data.point);
    } else {
    	append_to_file(DATA_DIR, "%d\t%s\t%d\t%s\t%s\n", data.id, data.name, data.age, data.class, data.point);
    }

    return 0;
}

static struct file_operations fops =
{
    .owner = THIS_MODULE,
    .open = usb_open,
    .release = usb_close,
    .read = usb_read,
    .write = usb_write,
};

static int usb_probe(struct usb_interface *interface, const struct usb_device_id *id)
{
    int retval;

    device = interface_to_usbdev(interface);

    class.name = "usb/huutung%d";
    class.fops = &fops;
    if ((retval = usb_register_dev(interface, &class)) < 0)
    {
        /* Something prevented us from registering this driver */
        printk(KERN_ERR "Not able to get a minor for this device.");
    }
    else
    {
        printk(KERN_INFO "Minor obtained: %d\n", interface->minor);
    }

    return retval;
}

static void usb_disconnect(struct usb_interface *interface)
{
    usb_deregister_dev(interface, &class);
}

static struct usb_device_id usb_table[] =
{
    { USB_DEVICE(USB_VENDOR_ID, USB_PRODUCT_ID) },
    {} /* Terminating entry */
};
MODULE_DEVICE_TABLE (usb, usb_table);

static struct usb_driver usb_driver =
{
    .name = "huuTung driver",
    .id_table = usb_table,
    .probe = usb_probe,
    .disconnect = usb_disconnect,
};

static int __init usb_init(void)
{
	int result;
	result = usb_register(&usb_driver);
	if (result) {
		printk("Register Error: %d", result);
		return -result;
	}
	printk("Register success: %d", result);
    return 0;
}

static void __exit usb_exit(void)
{
    usb_deregister(&usb_driver);
}

module_init(usb_init);
module_exit(usb_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Huu tung 09");
MODULE_DESCRIPTION("USB Registration Driver");
