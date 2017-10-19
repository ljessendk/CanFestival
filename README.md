This is a fork of the CanFestival-3 project http://dev.automforge.net/CanFestival-3

Latest work done:

- Fix some big endian issues and remove compiler warnings by adding explicit casts. Thanks to Casey Klimasuskas for sharing.

- The Canopen dictionary editor Objdictedit.py now allows to define the size of each string or domain, thanks to Mattes Standfuﬂ for his work 

- The stack can now be compiled as a .so shared lib, thanks to Mattes Standfuﬂ for this also

- New example added : examples/linux/dcf

- I needed the stack to be more dynamic, i wanted to be able to dynamically build the OD and the CO_Data struct without any global declaration, so i have made few changes. (this is not a dirty hack it is even cleaner i think)

- solving array of string or domain issue (search for "Array of strings issue" in the mailing list)

- solving bugs on sdo block transfer and dcf management

- stm32F0/F1/F4 basic support

Any feedback, comment, is welcome.

You can contact me at : 
fbeaulier < a t > orange < d o t > fr