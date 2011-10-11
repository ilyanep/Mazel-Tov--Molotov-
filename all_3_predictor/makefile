standalone_all_3: all_3_predictor.o standalone_all_3.o ../write_data/write_results.o
	g++ all_3_predictor.o standalone_all_3.o ../write_data/write_results.o -o standalone_all_3

all_3_predictor.o: all_3_predictor.cpp
	g++ -c all_3_predictor.cpp

standalone_all_3.o: standalone_all_3.cpp
	g++ -c standalone_all_3.cpp

../write_data/write_results.o::
	@$(MAKE) -C $(@D) $(@F) -q || \
	 $(MAKE) -C $(@D) $(@F)

clean:
	rm *.o standalone_all_3
