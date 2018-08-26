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

(datatype maybe
  X : A;
  ======================
  [just X] : (maybe A);

  ____________________
  nothing : (maybe A);)


(define divide
  {(A --> boolean) --> (list A) --> (list A) -->
   (list A) --> ( (maybe (list A)) * (maybe (list A)))}
  _ [] [] [] ->
  (@p nothing nothing)
  _ [] [] No ->
  (@p nothing [just No])
    _ [] Yes [] ->
  (@p [just Yes] nothing)
  _ [] Yes No ->
  (@p [just Yes] [just No])
  F [X | Y] Yes No ->
  (if (F X)
      (divide F Y
              [X | Yes] No)
      (divide F Y
              Yes [X | No])))

(define partition
  {(A --> boolean) --> (list A) --> ((maybe (list A))
                                     * (maybe (list A)))}
  _ [] -> (@p nothing nothing)
  R Elements ->
  (divide R
          Elements
          []
          []))
