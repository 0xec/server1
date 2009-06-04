#ifndef NOTIFIER_HPP_
#define NOTIFIER_HPP_
#include "base/base.hpp"
#include <boost/bind.hpp>
#include <boost/thread/condition.hpp>
#include <boost/thread/mutex.hpp>
class Notifier : public boost::enable_shared_from_this<Notifier>, public boost::noncopyable {
 public:
  Notifier(const string name, int initial_count = 1) : name_(name), count_(initial_count), notified_(false) {
  }
  const boost::function0<void> notify_handler() {
    return boost::bind(&Notifier::Notify, shared_from_this());
  }
  void Notify() {
    Dec(1);
  }
  void Inc(int cnt) {
    Dec(-1 * cnt);
  }
  void Dec(int cnt) {
    boost::mutex::scoped_lock locker(mutex_);
    count_ -= cnt;
    if (count_ == 0) {
      notify_.notify_all();
      notified_ = true;
      VLOG(2) << name_ << " : " << "Notifed";
    }
  }
  // Return true when notified, otherwise return false.
  bool Wait() {
    return Wait(LONG_MAX);
  }
  bool Wait(int timeout_ms) {
    while (!notified_) {
      boost::mutex::scoped_lock locker(mutex_);
      VLOG(2) << name_ << " : " << "Wait";
      bool ret = notify_.timed_wait(
          locker, boost::posix_time::milliseconds(timeout_ms));
      VLOG(2) << name_ << " : " << "Wait return";
      return ret;
    }
    return true;
  }
 private:
  int count_;
  bool notified_;
  boost::mutex mutex_;
  boost::condition notify_;
  string name_;
};
#endif // NOTIFIER_HPP_