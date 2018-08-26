(datatype set

   _____________
   [] : (set A);

   X : A; Y : ((set A) & (without X));
   ===================================
   [X | Y] : (mode (set A) -);

   X : (set A);
   _____________
   X : (list A);

   X : P; X : Q;
   ====================
   X : (mode (P & Q) -);

   ________________
   [] : (without X);

   if (not (= X Z))
   Y : (without Z);
   ______________________
   [X | Y] : (without Z);

   ______________________________________________
   (not (element? X Y)) : verified >> Y : (without X);)

(define foldl
  F Acc [] -> Acc
  F Acc [X | Rest] -> (foldl F (F Acc X) Rest))

(load "ml-types.shen")
(load "helpers.shen")
(load "types.shen")
(tc +)

(load "rho-grammar.shen")


(set n0 [quote zero])
(set n2 [quote [eval [quote [eval (value n0)]]]])

(set p1 [par [[eval (value n2)]
              zero]])

(set p2 [input [action (value n0)
                       (value n2)]
               (value p1)])

(set p3 [input [action (value n2)
                       (value n0)]
               (value p1)])

(deBruijnify (value p2) 0 0 0)
(deBruijnify (value p3) 0 0 0)
