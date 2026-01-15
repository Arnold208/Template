:: Copyright (c) Microsoft Corporation.
:: Licensed under the MIT License.

setlocal
cd /d %~dp0\..

openocd -f board/stm32f4discovery.cfg -c "program build/app/mxchip_threadx.elf verify reset exit"