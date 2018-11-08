; (C) 2018 Srimanta Barua
;
; This file defines the end of the _init and _fini functions which
; initialize and then shut down the C runtime. They are required for us
; to use C

; The .init section contains the _init function
section .init
        ; GCC will put the contents of crtend.o's .init section here
        ; The ending of the _init function
        pop	rbp
	ret

; The .fini section contains the _fini function
section .fini
        ; GCC will put the contents of crtend.o's .fini section here
        ; The ending of the _fini function
        pop	rbp
	ret

