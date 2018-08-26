(define guard
  {name --> name --> action}
  NSubj NObj -> [action NSubj NObj])

(define ->input
  {name --> name --> process --> process}
  NSubj NObj Cont -> [input [action NSubj NObj]
                            Cont])

(define prefix
  {action --> process --> process}
  [action [quote Proc1]
          [quote Proc2]] Cont ->
          [input [action
                  [quote
                   Proc1]
                  [quote Proc2]]
                 Cont])

(define ->lift
  {name --> process --> process}
  NSubj Cont -> [output NSubj Cont])

(define ->eval
  {name --> process}
  N -> [eval N])

(define ->par
  {process --> process --> process}
  [par Proclist1] [par Proclist2] -> [par (append Proclist1 Proclist2)]
  [par LProclist] Proc -> [par (append LProclist [Proc])]
  Proc [par RProclist] -> [par (cons Proc RProclist)]
  Proc1 Proc2 -> [par [Proc1 Proc2]])

(define parstar
  {(list process) --> process}
  [] -> zero
  [Proclisthd | Proclisttl] ->
  (->par Proclisthd (parstar Proclisttl)))

(define process-quote-depth
  {process --> number}
  zero -> 0
  [input [action NSubj _] Cont] ->
  (let
      QDSubj (name-quote-depth
              NSubj)
      QDCont (process-quote-depth Cont)

    (if (>= QDSubj QDCont)
        QDSubj
        QDCont))
  [output NSubj Cont] ->
  (let
      QDSubj (name-quote-depth NSubj)
      QDCont (process-quote-depth Cont)

    (if (>= QDSubj QDCont)
        QDSubj
        QDCont)))

(define name-quote-depth
  {name --> number}
  [quote Proc] ->
  (+ 1
     (process-quote-depth Proc)))


\* (datatype rho-compare-process *\
\*   __________________________________________ *\
\*   quote-depth-ordered-process : rho-compare-process; *\

\*   __________________________________________ *\
\*   unordered-process : rho-compare-process;) *\

\* (datatype rho-compare-process-name *\

\*   __________________________________________ *\
\*   quote-depth-ordered-process-name : rho-compare-name; *\
\*   __________________________________________ *\
\*   unordered-process-name : rho-compare-name; *\

\*   ) *\


\* (define compare-proc *\
\*   {rho-compare-process *\
\*    --> *\
\*    process *\
\*    --> *\
\*    process *\
\*    --> number} *\

\*   quote-depth-ordered-process *\
\*   Proc1 *\
\*   Proc2 -> *\
\*   (let *\
\*       Qdp1 (process-quote-depth Proc1) *\
\*       Qdp2 (process-quote-depth Proc2) *\
\*     (if (< Qdp1 Qdp2) *\
\*         -1 *\
\*         (if (= Qdp1 Qdp2) *\
\*             0 *\
\*             1))) *\
\*   unordered-process *\
\*   Proc1 *\
\*   Proc2 -> *\
\*   (let *\
\*       Qdp1 (process-quote-depth Proc1) *\
\*       Qdp2 (process-quote-depth Proc2) *\
\*     (if (= Proc1 Proc2) *\
\*         0 *\
\*         (if (< Qdp1 Qdp2) *\
\*             -1 *\
\*             1)))) *\

\* elements in lhs not in rhs *\



(define processes->names
  {(list process) --> (list name)}
  [] -> [[quote zero]]
  [Prochd | Proctl] -> (union (free-names Prochd)
                              (processes->names Proctl)))

(define free-names
  {process --> (list name)}
  zero -> []
  [input [action NSubj NObj] Cont] -> (cons
                                       NSubj
                                       (difference
                                        (free-names Cont)
                                        [NObj]))
  [output NSubj Cont] -> (cons NSubj
                             (free-names Cont))
  [eval Name] -> [Name]
  [par Processes] -> (processes->names Processes))

(define processes->par
  {(list process) --> process}
  [] -> zero
  [Prochd | Proctl] -> (->par Prochd (processes->par Proctl)))

(define calculate-next-name
  {process --> name}
  zero ->
  [quote zero]
  [input [action [quote PSubj] [quote PObj]] Cont] ->
  [quote (parstar [PSubj PObj Cont])]
  [output [quote PSubj] Cont] ->
  [quote (->par PSubj Cont)]
  [eval [quote Proc]] ->
  [quote (->par Proc Proc) ]
  [par Processes] ->
  [quote (processes->par Processes)]
  [par []] ->
  [quote zero])


(define syntactic-substitution
  {process --> name --> name --> process}
  zero _ _ -> zero
  [input [action NSubj NObj] Cont] NSource NTarget ->
  (let
      Obj (if (name-equivalent NObj NTarget)
               (calculate-next-name [input [action NSubj NObj] Cont])
               NObj)
      N0 (if (name-equivalent NSubj NTarget)
             NSource
             NSubj)
      Contt (syntactic-substitution
            (if (name-equivalent NObj NTarget)
                (syntactic-substitution Cont Obj NObj)
                Cont)
            NSource
            NTarget)
    [input [action N0 Obj] Contt])
  [output NSubj Cont] NSource NTarget ->
  [output (if (name-equivalent NSubj NTarget)
              NSource
              NSubj)
        (syntactic-substitution Cont NSource NTarget)]
  [eval Name] NSource NTarget ->
  [eval (if (name-equivalent Name NTarget)
            NSource
            Name)]
  [par Proclist] NSource NTarget ->
  [par (map (/. Proc
                (syntactic-substitution
                 Proc NSource NTarget))
            Proclist)])

(define alpha-equivalent
  {process --> process --> boolean}
  [input [action NSubj1 NObj1] Cont1]
  [input [action NSubj2 NObj2] Cont2] ->
  (and (name-equivalent NSubj1 NSubj2)
       (= Cont1 (syntactic-substitution Cont2 NObj1 NObj2)))
  Proc1 Proc2 -> (= Proc1 Proc2))

(define divide
  {(A --> boolean) --> (list A) --> (list A) -->
   (list A) --> ( (list A) * (list A))}
  _ [] Yes No ->
  (@p Yes No)
  F [X | Y] Yes No ->
  (if (F X)
      (divide F Y
              [X | Yes] No)
      (divide F Y
              Yes [X | No])))

(define partition
  {(A --> boolean) --> (list A) --> (list (list A))}
  _ [] -> []
  R [X | Y] ->
  (let Divide (divide R
                      [X | Y] [] [])
       Yes (fst Divide)
       No (snd Divide)
    [Yes | (partition R No)]))

\* 0 is the identity for part *\
(define structurally-equivalent-helper3
  {(list (list process)) --> boolean}
  [[] NotEquiv] -> false
  [[SingleEquivProc] NotEquiv] -> (structurally-equivalent zero
                                                           [par NotEquiv])
  [[Equivhd | Equivtail] _] -> false)

(define structurally-equivalent-helper2
  {(list process) --> (list process)  -->
   (boolean * process * (list process)) -->
   (list process) --> boolean}
  _ _ (@p Answer _ _) [] -> Answer
  Proclisttl1 ProcsNotEquiv (@p false R L) [Prochd | Proctl] ->
  (if (structurally-equivalent
       (parstar (cons R
                      (append L ProcsNotEquiv)))
       [par Proclisttl1])
      (structurally-equivalent-helper2
       Proclisttl1 ProcsNotEquiv (@p true R L) Proctl)
       (structurally-equivalent-helper2
       Proclisttl1 ProcsNotEquiv (@p false zero (tail ProcsNotEquiv)) Proctl))

  Proclisttl1 ProcsNotEquiv (@p true R L) [_ | Proctl] ->
  (structurally-equivalent-helper2
   Proclisttl1 ProcsNotEquiv
   (@p true R L) Proctl))

(define structurally-equivalent-helper1
  { (list process) --> (list (list process)) --> boolean}
  _ [[] No] -> false
  Proclisttl1 [YesEquiv NotEquiv] -> (structurally-equivalent-helper2
                                      Proclisttl1 NotEquiv
                                      (@p false zero (tail YesEquiv)) YesEquiv))

(define structurally-equivalent
  {process --> process --> boolean}
  zero [par []] -> true
  [par []] zero -> true
  zero [par [Proclisthd | Proclisttl]] ->
  (and (structurally-equivalent zero Proclisthd)
       (structurally-equivalent zero [par Proclisttl]))
  [par [Proclisthd | Proclisttl]] zero ->
  (structurally-equivalent zero [par (cons Proclisthd Proclisttl)])
  [input [action NSubj1 NObj1] Cont1] [input [action NSubj2 NObj2] Cont2] ->
  (and (name-equivalent NSubj1 NSubj2)
       (structurally-equivalent Cont1
                                (syntactic-substitution Cont2 NObj1 NObj2)))
  \* par is commutative and associative' *\
  [par [Proclisthd1 | Proclisttl1 ]] [par Proclist2] ->
  (structurally-equivalent-helper1 Proclisttl1 (partition
                                                (/. Proc (structurally-equivalent
                                                          Proclisthd1 Proc))
                                                Proclist2))
  [par Proclist1] [par [Proclisthd2 | Proclisttl2 ]] ->
  (structurally-equivalent [par [Proclisthd2 | Proclisttl2]] [par Proclist1])
  Proc1 [par Proclist] -> (structurally-equivalent-helper3 (partition
                                                            (/. Proc
                                                                (structurally-equivalent Proc1 Proc))
                                                            Proclist))
  [par Proclist] Proc2 ->
  (structurally-equivalent Proc2 [par Proclist])
  Proc1 Proc2 -> (= Proc1 Proc2))


(define name-equivalent
  {name --> name --> boolean}
  [quote [eval N1]] N2 -> (name-equivalent N1 N2)
  N1 [quote [eval N2]] -> (name-equivalent N1 N2)
  [quote P1] [quote P2] -> (structurally-equivalent P1 P2)
  [address Debruijnidx1] [address Debruijnidx2] ->
  (= Debruijnidx1 Debruijnidx2))


(define substitute
  {name --> name --> process --> process}
  _ _ zero -> zero
  Y X [input [action A B] Q] ->
  (let A' (if (name-equivalent A X)
              Y
              A)
       B' (if (name-equivalent B X)
              [quote [par [[eval B] Q]]]
              B)
       Q'' (if (name-equivalent B X)
               (substitute B' B Q)
               Q)
       Q' (substitute Y X Q)
    [input [action A' B'] Q'])
  Y X [output A Q] ->
  (let A' (if (name-equivalent A X)
              Y
              A)
       Q'  (substitute Y X Q)
    [output A' Q'])
  Y X [par Proclist] ->
  (let Proclist' (map (/. Proc
                          (substitute Y X Proc))
                      Proclist)
    [par Proclist'])
  Y X [eval A] ->
  (let A' (if (name-equivalent A X)
              Y
              A)
    [eval A']))

(define deBruijnify
  {process --> number --> number --> number --> process}
  zero L W H -> zero

  [input [action [quote Px] Y] Q] L W H ->
  (let Dbnidx (to-number
               (append
                (to-bits L)
                (to-bits W)
                (to-bits H)))
       Dbny [address Dbnidx]
       Q'  (deBruijnify Q (+ L 1) W H)
       Q'' (substitute Dbny Y Q')
       X  [quote (deBruijnify Px L W (+ H 1))]
    [input [action X Dbny] Q''])
  [output [quote Px] Q] L W H ->
  (let X [quote (deBruijnify Px L W (+ 1 H))]
       Q' (deBruijnify Q L W H)
    [output X Q'])

  [par [Proclisthd | Proclisttl]] L W H ->
  [par (cons
        (deBruijnify Proclisthd L W H)
        (map (/. Proc
                  (deBruijnify Proc L W (+ H 1)))
             Proclisttl))]

  [eval [quote Px]] L W H ->
  (let X [quote (deBruijnify Px L W (+ H 1))]
    [eval X])

  [eval [address Addr]] L W H ->
  [eval [address Addr]])
