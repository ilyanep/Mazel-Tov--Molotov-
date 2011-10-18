run_blender: run_blender.o blender.o ../write_data/write_results.o ../binary_files/binary_files.o ../SVD/learn_svd.o ../Baseline/baseline_predictor.o
	g++ -lgsl -lgslcblas -lm run_blender.o blender.o ../write_data/write_results.o ../binary_files/binary_files.o ../SVD/learn_svd.o ../Baseline/baseline_predictor.o -o run_blender

run_blender.o: run_blender.cpp 
	g++ -lgsl -lgslcblas -lm -c run_blender.cpp

blender.o: blender.cpp
	g++ -lgsl -lgslcblas -lm -c blender.cpp

../write_data/write_results.o::
	@$(MAKE) -C $(@D) $(@F) -q || \
	$(MAKE) -C $(@D) $(@F)

../binary_files/binary_files.o::
	@$(MAKE) -C $(@D) $(@F) -q || \
	$(MAKE) -C $(@D) $(@F)

../SVD/learn_svd.o::
	@$(MAKE) -C $(@D) $(@F) -q || \
	$(MAKE) -C $(@D) $(@F)

../Baseline/baseline_predictor.o::
	@$(MAKE) -C $(@D) $(@F) -q || \
	$(MAKE) -C $(@D) $(@F)

clean:
	rm *.o run_blender
