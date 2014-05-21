/*
 *author: 李瀚
 *create date : 060719
 */
#ifndef __H__LOCK
#define __H__LOCK

#include "pthread.h"
#include "assert.h"
/*
 *采用写者优先的读写锁
 */
class Cwp_rwlock
{
public:
	Cwp_rwlock()
	{
		m_rc = 0;
		m_wc = 0;
		pthread_mutex_init(&m_wc_lock,NULL);
		pthread_mutex_init(&m_rc_lock,NULL);
		pthread_mutex_init(&m_w_lock,NULL);
		pthread_mutex_init(&m_r_lock,NULL);
	}
	inline void writer_enter()
	{
		pthread_mutex_lock(&m_wc_lock);
		m_wc++;
		if(1 == m_wc){
				pthread_mutex_lock(&m_r_lock);
		}
		pthread_mutex_unlock(&m_wc_lock);
		pthread_mutex_lock(&m_w_lock);
	}
	inline void writer_leave()
	{
		pthread_mutex_unlock(&m_w_lock);
		pthread_mutex_lock(&m_wc_lock);
		if(m_wc>0){
				m_wc--;
		}
		if(0 == m_wc){
				pthread_mutex_unlock(&m_r_lock);
		}
		pthread_mutex_unlock(&m_wc_lock);
	}
	inline void reader_enter()
	{
		pthread_mutex_lock(&m_r_lock);
		pthread_mutex_lock(&m_rc_lock);
		m_rc++;
		if(1==m_rc){
			pthread_mutex_lock(&m_w_lock);
		}	
		pthread_mutex_unlock(&m_rc_lock);
		pthread_mutex_unlock(&m_r_lock);
	}
	
	inline int reader_tryenter()
	{
		int ret=-1;
		ret = pthread_mutex_trylock(&m_r_lock);
		if(0!=ret){
			return ret;
		}
		pthread_mutex_lock(&m_rc_lock);
		m_rc++;
		if(1 == m_rc){
			ret = pthread_mutex_trylock(&m_w_lock);
		}
		if(0 != ret){
			m_rc--;
		}
		pthread_mutex_unlock(&m_rc_lock);
		pthread_mutex_unlock(&m_r_lock);
		return ret;
	}
	inline void reader_leave()
	{
		pthread_mutex_lock(&m_rc_lock);
		if(m_rc>0){
			m_rc--;
		}
		if(0==m_rc){
			pthread_mutex_unlock(&m_w_lock);
		}
		pthread_mutex_unlock(&m_rc_lock);
	}
private:
		int m_wc,m_rc;
		pthread_mutex_t m_wc_lock,m_rc_lock,m_w_lock,m_r_lock;//读者计数和写者锁
};
/*
 *普通锁的封装类，在构造函数上锁，在析构函数解锁
 */
class Clock_wrapper
{
public:
	Clock_wrapper(pthread_mutex_t* lock)
	{	
	//	ul_writelog(UL_LOG_DEBUG,"construct Clock:before enter mutex lock");
	//	ul_writelog(UL_LOG_NOTICE,"construct Clock:before enter mutex lock");
		m_p_lock = lock;
		pthread_mutex_lock(m_p_lock);
	}
	~Clock_wrapper()
	{
		pthread_mutex_unlock(m_p_lock);
		//	ul_writelog(UL_LOG_DEBUG,"destruct Clock");
	}
	private:
		pthread_mutex_t *m_p_lock;
};
/*
 *读写锁的封装类，在构造函数上锁，在析构解锁
 */
typedef void (* PFUNC_MUTEX )(Cwp_rwlock*);
class Crwlock_wrapper
{
public:

	enum CALLER_TYPE
	{
		WRITER=0,
		READER=1
	};
	Crwlock_wrapper(Cwp_rwlock* lock,CALLER_TYPE caller_type)
	{
		assert((WRITER == caller_type) || (READER ==caller_type));
		m_rwlock = lock;
		m_caller_type=caller_type;
		m_lock_func[m_caller_type](m_rwlock);
	}
	~Crwlock_wrapper()
	{
		m_unlock_func[m_caller_type](m_rwlock);
	}
	static inline void writer_enter(Cwp_rwlock* lock)
	{
		lock->writer_enter();
	}
	static inline void writer_leave(Cwp_rwlock* lock)
	{
		lock->writer_leave();
	}
	static inline void reader_enter(Cwp_rwlock* lock)
	{
		lock->reader_enter();
	}
	static inline void reader_leave(Cwp_rwlock* lock)
	{
		lock->reader_leave();
	}
private:
	static const PFUNC_MUTEX m_lock_func[2];
	static const PFUNC_MUTEX m_unlock_func[2];
	Cwp_rwlock* m_rwlock;
	CALLER_TYPE m_caller_type;
};
#endif
