;; C runtime
;; This file defines the end of the _init and _fini functions which
;; initialize and then shut down the C runtime. They are required for us
;; to use C

[BITS 32]

;; The .init section contains the _init function
section .init
        ;; GCC will put the contents of crtend.o's .init section here
        ;; The ending of the _init function
        pop	ebp
	ret


;; The .fini section contains the _fini function
section .fini
        ;; GCC will put the contents of crtend.o's .fini section here
        ;; The ending of the _fini function
        pop	ebp
	ret

