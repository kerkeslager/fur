struct Frame;
typedef struct Frame Frame;
struct Frame {

  Environment* environment;
  Frame* returnFrame;
  size_t programCounter;
};

void Frame_initialize(Frame* self, Environment* environment, Frame* returnFrame, size_t programCounter) {
  self->environment = environment;
  self->returnFrame = returnFrame;
  self->programCounter = programCounter;
}

void Frame_deinitialize(Frame* self) {
}
