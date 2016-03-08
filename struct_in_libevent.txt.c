struct event {
    /*
    ev_next 就是该 I/O 事件在链表中的位置；称此链表为“已注册事件链表”；
    同样 ev_signal_next 就是 signal 事件在 signal 事件链表中的位置；
    ev_active_next：libevent 将所有的激活事件放入到链表 active list 中，然后遍历 active list 执行调度，ev_active_next 就指明了 event 在 active list 中的位置；
    */
    TAILQ_ENTRY (event) ev_next;
    TAILQ_ENTRY (event) ev_active_next;
    TAILQ_ENTRY (event) ev_signal_next;
    /*
    min_heap_idx 和 ev_timeout，如果是 timeout 事件，它们是 event 在小根堆中的索引和超时值，libevent 使用小根堆来管理定时事件，
    */
    unsigned int min_heap_idx;  /* for managing timeouts */

    /* 该事件所属的反应堆实例，这是一个 event_base 结构体 */
    struct event_base *ev_base;

    /* 对于 I/O 事件，是绑定的文件描述符；对于 signal 事件，是绑定的信号 */
    int ev_fd;
    short ev_events;    // I/O事件： EV_WRITE和EV_READ
                        // 定时事件：EV_TIMEOUT
                        // 信号：    EV_SIGNAL
                        // 辅助选项：EV_PERSIST，表明是一个永久事件
                        // 信号和I/O事件不能同时设置
    /*
    事件就绪执行时，调用 ev_callback 的次数，通常为 1
    */
    short ev_ncalls;
    /* 通常指向 ev_ncalls 或者为 NULL */
    short *ev_pncalls;  /* Allows deletes in callback */

    struct timeval ev_timeout;

    int ev_pri;     /* smaller numbers are higher priority */

    /*
    event 的回调函数，被 ev_base 调用，执行事件处理程序，这是一个函数指针，原型为：
    void (*ev_callback)(int fd, short events, void *arg)
    其中参数 fd 对应于 ev_fd；events 对应于 ev_events；arg 对应于 ev_arg
    */
    void (*ev_callback)(int, short, void *arg);
    /* 表明可以是任意类型的数据，在设置 event 时指定 */
    void *ev_arg;

    /* 记录了当前激活事件的类型 */
    int ev_res;     /* result passed to event callback */

    /*
    libevent 用于标记 event 信息的字段，表明其当前的状态，可能的值有：
    #define EVLIST_TIMEOUT 0x01 // event在time堆中
    #define EVLIST_INSERTED 0x02 // event在已注册事件链表中
    #define EVLIST_SIGNAL  0x04 // 未见使用
    #define EVLIST_ACTIVE  0x08 // event在激活链表中
    #define EVLIST_INTERNAL 0x10 // 内部使用标记
    #define EVLIST_INIT 0x80 // event 已被初始化
    */
    int ev_flags;
};

/*
要向 libevent 添加一个事件，需要首先设置 event 对象，这通过调用 libevent 提供的函数有：event_set(), event_base_set(), event_priority_set()来完成；下面分别进行讲解。
void event_set(struct event *ev, int fd, short events, void (*callback)(int, short, void *), void *arg)
    1.设置事件 ev 绑定的文件描述符或者信号，对于定时事件，设为-1 即可；
    2.设置事件类型，比如 EV_READ|EV_PERSIST, EV_WRITE, EV_SIGNAL 等；
    3.设置事件的回调函数以及参数 arg；
    4.初始化其它字段，比如缺省的 event_base 和优先级；
int event_base_set(struct event_base *base, struct event *ev)
    设置 event ev 将要注册到的 event_base；
    libevent 有一个全局 event_base 指针 current_base，默认情况下事件 ev 将被注册到 current_base 上，使用该函数可以指定不同的 event_base；
    如果一个进程中存在多个 libevent 实例，则必须要调用该函数为 event 设置不同的 event_base；
int event_priority_set(struct event *ev, int pri)
    设置event ev的优先级，没什么可说的，注意的一点就是：当ev正处于就绪状态时，不能设置，返回-1。
*/
