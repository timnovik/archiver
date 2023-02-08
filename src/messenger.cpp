#include "messenger.h"

Messenger::Messenger(std::ostream* stream_ptr) {
    stream_ptr_ = stream_ptr;
}

void Messenger::SendMessage(Message message) {
    (*stream_ptr_) << messages_[message];
}
