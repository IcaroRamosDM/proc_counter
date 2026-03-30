#include <linux/init.h>         // Macros __init e __exit
#include <linux/kernel.h>       // pr_info(), pr_err()
#include <linux/module.h>       // Infraestrutura de módulos do kernel
#include <linux/proc_fs.h>      // API de /proc
#include <linux/uaccess.h>      // copy_to_user(), copy_from_user()
#include <linux/string.h>       // strcmp(), strncmp(), strcspn()
#include <linux/mutex.h>        // mutex, mutex_lock, mutex_unlock

#define PROC_NAME "proc_counter"    // Nome do arquivo em /proc
#define BUFFER_SIZE 128             // Tamanho máximo de entrada do usuário

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Icaro");
MODULE_DESCRIPTION("Kernel module with /proc interface");
MODULE_VERSION("0.5");

// Ponteiro para a entrada criada em /proc
static struct proc_dir_entry *proc_entry;

/*
 * Estado interno do módulo.
 * Essas variáveis são compartlhadas entre diferentes acessos ao arquivo
 * em /proc, então pode-se protegê-las com uma mutex.
 */
static int read_count = 0;
static int write_count = 0;
static int stored_value = 0;

/* Mutex usada para proteger o estado interno do módulo */
static DEFINE_MUTEX(proc_counter_mutex);

/*
 * Função chamada quando o usuário faz:
 * cat /proc/proc_counter
 */
static ssize_t proc_read(struct file *file, char __user *user_buffer,
                        size_t count, loff_t *offset)
{
    char buffer[128];
    int len;
    int local_read_count;
    int local_write_count;
    int local_stored_value;

    // Evita leitura infinita
    if (*offset > 0)
        return 0;

    /*
     * Região crítica para atualizar e ler o estado de forma segura.
     */
    mutex_lock(&proc_counter_mutex);

    // Incrementa contador de leitura
    read_count++;

    /*
     * Copia os valores para as variáveis locais enquanto a mutex está travada.
     * Assim mantém-se consistência na saída.
     */
    local_read_count = read_count;
    local_write_count = write_count;
    local_stored_value = stored_value;

    mutex_unlock(&proc_counter_mutex);

    // Monta a mensagem que será enviada ao user space
    len = snprintf(buffer, sizeof(buffer),
                  "Read count: %d\n"
                  "Write count: %d\n"
                  "Stored value: %d\n",
                  local_read_count, local_write_count, local_stored_value);

    // Copia dados do kernel para o usuário
    if (copy_to_user(user_buffer, buffer, len))
        return -EFAULT;

    *offset = len; // Atualiza offset para indicar que já leu

    return len; // Retorna quantos bytes foram lidos
}

/*
 * Função chamada ao escrever em /proc/proc_counter
 * Exemplo:
 * echo reset > /proc/proc_counter
 */
static ssize_t proc_write(struct file *file, const char __user *user_buffer,
                          size_t count, loff_t *offset)
{
    char buffer[BUFFER_SIZE];
    int value;

    // Evita overflow
    if (count >= BUFFER_SIZE)
        return -EINVAL;

    // Copia dados do usuário para o kernel
    if (copy_from_user(buffer, user_buffer, count))
        return -EFAULT;

    buffer[count] = '\0'; // Garante string terminada

    buffer[strcspn(buffer, "\n")] = '\0'; // Remove \n que vem do comando de echo

    mutex_lock(&proc_counter_mutex);

    // Incrementa contador de escrita
    write_count++;

    /*
     * Verifica comandos
     */

    // Caso: reset
    if (strcmp(buffer, "reset") == 0) {
        read_count = 0;
        write_count = 0;
        stored_value = 0;

        mutex_unlock(&proc_counter_mutex);

        pr_info("proc_counter: reset command received\n");
        return count;
    }
    
    /*
     * Se não for "reset", tenta interpretar o conteúdo como inteiro.
     */
    if (kstrtoint(buffer, 10, &value) == 0) {
        stored_value = value;
        
        mutex_unlock(&proc_counter_mutex);

        pr_info("proc_counter: stored value updated to %d\n", stored_value);
        return count;
    }

    mutex_unlock(&proc_counter_mutex);

    /*
     * Se a entrada não for reset nem inteiro
     */
    pr_err("proc_counter: invalid input: \"%s\"\n", buffer);
    return -EINVAL;
}

/*
 * Estrutura com as operações do arquivo em /proc
 */
static const struct proc_ops proc_file_ops = {
    .proc_read = proc_read, // Associa a função de leitura
    .proc_write = proc_write, // Associa a função de escrita
};

/*
 * Inicialização do módulo
 */
static int __init proc_counter_init(void)
{
    /*
     * Cria o arquivo /proc/proc_counter
     */
    proc_entry = proc_create(PROC_NAME, 0666, NULL, &proc_file_ops);

    if (!proc_entry) {
        pr_err("proc_counter: failed to create /proc/%s\n", PROC_NAME);
        return -ENOMEM;
    }

    pr_info("proc_counter: module loaded\n");
    return 0;
}

/*
 * Finalização do módulo
 */
static void __exit proc_counter_exit(void)
{
    /*
     * Remove o arquivo de /proc
     */
    proc_remove(proc_entry);
    pr_info("proc_counter: module unloaded\n");
}

module_init(proc_counter_init);
module_exit(proc_counter_exit);

