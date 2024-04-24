front:
	@make -f MakeFrontend
	@echo '>>> make front - Success!'
back:
	@make -f MakeBackend
	@echo '>>> make back - Success!'

rfront:
	@make -f MakeReverseFrontend
	@echo '>>> make rfront - Success!'

clean:
	@make -f MakeFrontend clean
	@make -f MakeBackend clean
	@make -f MakeReverseFrontend clean


