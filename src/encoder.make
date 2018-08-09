# Common include for building the encoder

OBJ      = obj/$(ARCH)/cliparameters.o \
           obj/$(ARCH)/pcmstream.o \
           obj/$(ARCH)/encoder.o \
           obj/$(ARCH)/encode.o \

$(BIN): $(OBJ) Makefile.Encoder_$(MEXT)
	$(CXX) $(CXXFLAGS) $(OBJ) -o $(BIN)

obj/$(ARCH)/cliparameters.o: cliparameters.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

obj/$(ARCH)/pcmstream.o: pcmstream.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

obj/$(ARCH)/encoder.o: encoder.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

obj/$(ARCH)/encode.o: encode.cpp
	$(CXX) $(CXXFLAGS) -o $@ -c $<

clean:
	$(RM) $(TARGET) obj/$(ARCH)/*.o *~
