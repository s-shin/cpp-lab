#include <iostream>
#include <memory>
#include <thread>
#include "asio.hpp"

class Worker {
 public:
  Worker(asio::io_service* main_io)
      : thread_(), io_(), main_io_(main_io), work_(*main_io) {
    thread_ =
        std::unique_ptr<std::thread>(new std::thread([this]() { Run(); }));
  }

  int Run() {
    main_io_->post([this]() {
      std::cout << "Worker(" << thread_->get_id() << ") ::: Run" << std::endl;
    });
    io_.post(std::bind(&Worker::Do, this));
    io_.run();
    return 0;
  }

  void Do() {
    main_io_->post([this]() {
      std::cout << "Worker(" << thread_->get_id() << ") ::: Do" << std::endl;
    });
    std::this_thread::sleep_for(std::chrono::seconds(1));
    io_.post(std::bind(&Worker::Do, this));
  }

 private:
  std::unique_ptr<std::thread> thread_;
  asio::io_service io_, *main_io_;
  asio::io_service::work work_;
};

int main(int argc, char** argv) {
  asio::io_service main_io;
  Worker w1(&main_io), w2(&main_io);
  main_io.run();
  return 0;
}
