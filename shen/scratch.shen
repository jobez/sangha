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
              zero
              [eval [quote [eval [quote zero]]]]]])

(set p2 [input [action (value n0)
                       (value n2)]
               (value p1)])

(set p3 [input
         [action [quote [eval [quote [eval [quote zero]]]]]
                 [quote zero]]
         [output (value n2)
                 (value p2)]])

(structure-equivalent )

(deBruijnify (value p2) 0 0 0)
(deBruijnify (value p3) 0 0 0)
(track partition)
(track divide)
(track structurally-equivalent-helper1)
(track structurally-equivalent-helper2)
(track structurally-equivalent)
(track substitute)
(track syntactic-substitution)
(track calculate-next-name)

(structurally-equivalent
 [par [zero [eval [quote zero]] zero zero]]
 [par [[eval [quote zero]] zero zero zero]])

(structurally-equivalent [par [[eval [quote zero]]  zero zero]] [par [zero [eval [quote zero]] zero zero zero zero zero]])

(structurally-equivalent [par [zero zero zero zero]]
                         [par []])

(structurally-equivalent [par [zero [par [zero]]]]
                         [par [zero [par [zero]]]])

(free-names [par [zero [eval [quote zero]] zero zero]])

(substitute [input [action [address 0] [address 1]] [eval [quote zero]]]
            [quote [eval [quote zero]]]
            [address 1])

(substitute
 [input
  [action
   [quote zero]
   [quote [eval
           [quote
            [output
             [quote zero]
             zero]]]]]
        [eval
         [quote zero]]]

 [quote
  [eval
   [quote zero]]]

 [quote
  [eval
   [quote
    [output
     [quote zero]
     zero]]]])

[input [action [quote zero]
               [quote [par
                       [zero
                        [eval
                         [quote
                          [output
                           [quote zero]
                           zero]]]
                        [eval [quote zero]]
                        zero]
                       ] ]] [eval [quote zero]]]

(syntactic-substitution
 [input
  [action
   [quote zero]
   [quote [eval
           [quote
            [output
             [quote zero]
             zero]]]]]
  [eval
   [quote zero]]]

 [quote
  [eval
   [quote zero]]]

 [quote
  [eval
   [quote
    [output
     [quote zero]
     zero]]]])



[input [action [quote zero]
               [quote
                [par
                 [[eval [quote
                         [eval
                          [quote
                           [output
                            [quote zero]
                            zero]]]]]
                  [eval [quote zero]]]]]]
       [eval [quote zero]]]

(name-equivalent [quote [eval [quote [output [quote zero] zero]]]] [quote zero])
