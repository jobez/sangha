(lisp. "(DEFUN to-bits (INTEGER)
  (LET ((BITS '()))
    (DOTIMES (POS (INTEGER-LENGTH INTEGER) BITS)
      (PUSH (LDB (BYTE 1 POS) INTEGER) BITS))))")

(lisp. "(DEFUN to-number (NLIST)
  (IF NLIST
      (LET ((X (CAR NLIST))
            (Xs (CDR NLIST)))
        (+ (* (EXPT (1-  (LENGTH NLIST))
                    2)
              X)
           (to-number Xs)))
      0))" )
