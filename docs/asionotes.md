# General
## What is ASIO?

### What is io_context?
io_context is an abstraction for the Reactor.
Basically, we put tasks on the queue. They are executed upon being put and OS posts results of their succession. Their results is dequed only if we run io_context::run() -> this deques the result, and calls the completion handler, that was provided with the async task.
It can be thought as a scheduler.