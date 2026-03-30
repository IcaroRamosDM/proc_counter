
#include <linux/init.h>         // Necessario para macros __init e __exit
#include <linux/kernel.h>       // Necessario para pr_info()
#include <linux/module.h>       // Necessario para modulos do kernel
#include <linux/proc_fs.h>      // Necessario para trabalhar com /proc
#include <linux/uaccess.h>      // Necessario para copy_to_user()

#define PROC_NAME "proc_counter"    // Nome do arquivo em /proc

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Icaro");
MODULE_DESCRIPTION("Kernel module with /proc interface");
MODULE_VERSION("0.2");

// Ponteiro para a entrada criada em /proc
static struct proc_dir_entry *proc_entry;

/*
 * Função chamada quando o usuário faz:
 * cat /proc/proc_counter
 */
static ssize_t proc_read(struct file *file, char __user *user_buffer,
                        size_t count, loff_t *offset)
{
    char message[] = "Hello from kernel space!\n"; // Mensagem que vamos retornar
    int len = sizeof(message);

    /*
     * Esse if evita que o conteúdo seja lido várias vezes.
     * Sem isso, o cat entraria em loop infinito.
     */
    if (*offset > 0)
        return 0;

    /*
     * copy_to_user:
     * copia dados do espaço do kernel para o espaço do usuario
     */
    if (copy_to_user(user_buffer, message, len))
        return -EFAULT;

    *offset = len; // Atualiza offset para indicar que já leu

    return len; // Retorna quantos bytes foram lidos
}

/*
 * Estrutura que define as operações do arquivo em /proc
 */
static const struct proc_ops proc_file_ops = {
    .proc_read = proc_read, // Associa a função de leitura
};

/*
 * Função chamada quando o módulo é carregado
 */
static int __init proc_counter_init(void)
{
    pr_info("proc_counter: module loaded\n");

    /*
     * Cria o arquivo /proc/proc_counter
     */
    proc_entry = proc_create(PROC_NAME, 0, NULL, &proc_file_ops);

    if (!proc_entry) {
        pr_err("proc_counter: failed to create /proc entry\n");
        return -ENOMEM;
    }

    return 0;
}

/*
 * Função chamada quando o módulo é removido
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

