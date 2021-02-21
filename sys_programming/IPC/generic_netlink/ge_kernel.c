#include <net/genetlink.h>
#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/init.h>

#define NLA_DATA(na)		((void *)((char*)(na) + NLA_HDRLEN))


/*netlink attribute type:*/
enum {
	NAP_MAC_A_UNSPEC,
	NAP_MAC_A_MSG,
	__NAP_MAC_A_MAX,
};

#define NAP_MAC_A_MAX 	(__NAP_MAC_A_MAX - 1)

/* command type */
enum {
	NAP_MAC_CMD_UNSPEC,
	NAP_MAC_RECV_CMD,
	__NAP_MAC_CMD_MAX
};

#define NAP_MAC_CMD_MAX (__NAP_MAC_CMD_MAX - 1)

/* atribute policy */
static struct nla_policy nap_mac_genl_policy[NAP_MAC_A_MAX + 1] = {
	[NAP_MAC_A_MSG] = {.type = NLA_NUL_STRING},
};


#define VERSION_NR 	1

/* family definition */
static struct genl_family nap_mac_genl_family = {
	.id = GENL_ID_GENERATE, //  auto generate family ID
	.hdrsize = 0,
	.name = "NAP_MAC",
	.version = VERSION_NR,
	.maxattr = NAP_MAC_A_MAX,
};

static pid_t nap_pid;

/* Send data to nap */
static int nap_mac_send_genl_data(void *data, int len)
{
	struct sk_buff *skb;
    size_t size;
	void *msg_hdr;
	int rc = 0;

    size = nla_total_size(len);

	skb = genlmsg_new(size, GFP_KERNEL); //create a new netlink msg
	if (!skb)
		goto err_out;

	/*
	 * Add a new netlink message to an skb
	 */
    msg_hdr = genlmsg_put(skb, nap_pid, 0,
                    &nap_mac_genl_family, 0, NAP_MAC_RECV_CMD);
	if (!msg_hdr) {
		kfree_skb(skb);
		rc = -ENOMEM;
		goto err_out;
	}

	/*Fill in the specific netlink attribute:msg,
	 * which is the actual data to send
	 * */
	rc = nla_put(skb, NAP_MAC_A_MSG, len, data);
	if (rc) {
		kfree_skb(skb);
		goto err_out;
	}

	/*A process that is sent to a user space by a unicast */
	rc = genlmsg_unicast(&init_net, skb, nap_pid);
	if (rc)
		goto err_out;

	return 0;
err_out:
	printk(KERN_DEBUG"%d, %s: genlmsg_unicast error pid = %u, rc = %d\n",
	       __LINE__, __func__, nap_pid, rc);
	return rc;
}


/* doit handler */
static int nap_mac_genl_recv_data(struct sk_buff *skb, struct genl_info *info)
{
    struct nlmsghdr     *nlhdr;
    struct genlmsghdr   *genlhdr;
    struct nlattr       *nla;
    char *data_buf;
    size_t data_len;

    nlhdr = nlmsg_hdr(skb);
    nap_pid = nlhdr->nlmsg_pid;

    genlhdr = nlmsg_data(nlhdr);
    nla = genlmsg_data(genlhdr);
    data_buf = (char *)NLA_DATA(nla);
    data_len = nla_len(nla);

    printk("%s\n", data_buf);
    printk("%d\n", data_len);

    nap_mac_send_genl_data(data_buf, data_len);

	return 0;
}


static struct genl_ops nap_mac_genl_ops_list[] = {
	{
		.cmd = NAP_MAC_RECV_CMD,
		.flags = 0,
		.policy = nap_mac_genl_policy,
		.doit = nap_mac_genl_recv_data,
		.dumpit = NULL,
	}
};

/* register generic netlink family and operation */
static int nap_mac_genl_kernel_init(void)
{
	int rtval = 0;

	rtval = genl_register_family_with_ops(&nap_mac_genl_family,
					      nap_mac_genl_ops_list);
	if (rtval) {
		genl_unregister_family(&nap_mac_genl_family);
		goto err_out;
	}

    printk(KERN_ERR "generic netlink register success....\n");

	return 0;

err_out:
	printk(KERN_DEBUG "cap_smd: genl_kernel_init error!\n");
	return -1;
}

static void nap_mac_genl_kernel_release(void)
{
	int rtval;

	rtval = genl_unregister_family(&nap_mac_genl_family);
    printk(KERN_ERR "generic netlink unregister....\n");

	if (rtval)
		printk(KERN_DEBUG "cap_smd: genl_kernel_release error!\n");
}

module_init(nap_mac_genl_kernel_init);
module_exit(nap_mac_genl_kernel_release);
MODULE_AUTHOR("wangyong");
MODULE_LICENSE("GPL");

