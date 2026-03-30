#include <linux/init.h>         // Necessario para macros __init e __exit
#include <linux/kernel.h>       // Necessario para pr_info()
#include <linux/module.h>       // Necessario para modulos do kernel
#include <linux/proc_fs.h>      // Necessario para trabalhar com /proc
#include <linux/uaccess.h>      // Necessario para copy_to_user()
#include <linux/string.h>       // Para strcmp

#define PROC_NAME "proc_counter"    // Nome do arquivo em /proc
#define BUFFER_SIZE 128             // Tamanho máximo de entrada do usuário

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Icaro");
MODULE_DESCRIPTION("Kernel module with /proc interface");
MODULE_VERSION("0.3");

// Ponteiro para a entrada criada em /proc
static struct proc_dir_entry *proc_entry;

// Variáveis internas do módulo (estado)
static int read_count = 1;
static int write_count = 0;
static int stored_value = 0;

/*
 * Função chamada quando o usuário faz:
 * cat /proc/proc_counter
 */
static ssize_t proc_read(struct file *file, char __user *user_buffer,
                        size_t count, loff_t *offset)
{
    char buffer[128];
    int len;

    // Monta a mensagem que será enviada ao user space
    len = snprintf(buffer, sizeof(buffer),
                  "Read count: %d\nWrite count: %d\nStored value: %d\n",
                  read_count, write_count, stored_value);

    // Evita leitura infinita
    if (*offset > 0)
        return 0;
    
    // Incrementa contador de leitura
    read_count++;

    // Copia dados do kernel para o usuário
    if (copy_to_user(user_buffer, buffer, len))
        return -EFAULT;

    *offset = len; // Atualiza offset para indicar que já leu

    return len; // Retorna quantos bytes foram lidos
}

/*
 * FUnção chamada ao escrever em /proc/proc_counter
 * Exemplo:
 * echo reset > /proc/proc_counter
 */
static ssize_t proc_write(struct file *file, const char __user *user_buffer,
                          size_t count, loff_t *offset)
{
    char buffer[BUFFER_SIZE];

    // Evita overflow
    if (count > BUFFER_SIZE -1)
        return -EINVAL;

    // Copia dados do usuário para o kernel
    if (copy_from_user(buffer, user_buffer, count))
        return -EFAULT;

    buffer[count] = '\0'; // Garante string terminada

    // Incrementa contador de escrita
    write_count++;

    /*
     * Verifica comandos
     */

    // Caso: reset
    if (strncmp(buffer, "reset", 5) == 0) {
        read_count = 1;
        write_count = 0;
        stored_value = 0;
        pr_info("proc_counter: reset command received\n");
    }
    else {
        /*
         * Tenta interpretar como número
         * Ex: echo 42 > /proc/proc_counter
         */
        int value;
        if (kstrtoint(buffer, 10, &value) == 0) {
            stored_value = value;
            pr_info("proc_counter: stored value updated to %d\n", value);
        } else {
            pr_info("proc_counter: invalid input\n");
        }
    }

    return count;
}

/*
 * Estrutura com as operações do arquivo em /proc
 */
static const struct proc_ops proc_file_ops = {
    .proc_read = proc_read, // Associa a função de leitura
    .proc_write = proc_write, // Associa a função de escrita
};

/*
 * INicialização do módulo
 */
static int __init proc_counter_init(void)
{
    pr_info("proc_counter: module loaded\n");

    /*
     * Cria o arquivo /proc/proc_counter
     */
    proc_entry = proc_create(PROC_NAME, 0666, NULL, &proc_file_ops);

    if (!proc_entry) {
        pr_err("proc_counter: failed to create /proc entry\n");
        return -ENOMEM;
    }

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

