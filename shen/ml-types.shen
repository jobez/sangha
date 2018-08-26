(defcc <ml-rules>
  <ml-rule> , <ml-rules> := [<ml-rule> | <ml-rules>];
  <ml-rule> := [<ml-rule>];)

(defcc <ml-rule>
  Constructor of <datatypes> := [Constructor | <datatypes>];
  <datatype> := <datatype>;)

(defcc <datatypes>
  <datatype> <datatypes> := [ <datatype> | <datatypes>];
  <e>;)

(defcc <datatype>
  Datatype := Datatype where (not (= Datatype ,));)

(package ml-types [<datatype> <datatypes> <ml-rule> <ml-rules> ml-datatype]

(define parse-ml
  ML-Rules -> (compile (function <ml-rules>)
                       ML-Rules))
(define ml->shen
  Datatype [Constructor | Datatypes] ->
  (let
      Vars (map (function var) Datatypes)
      Conclusion (conclusion Constructor Datatype Vars)
      Premises (premises Vars Datatypes)
    (append Premises [======] Conclusion))

  Datatype Instance -> [__________________
                        Instance : Datatype;])

(define conclusion
  Constructor Datatype Vars ->
  [(cons-form [Constructor | Vars]) : Datatype;])

(define premises
  [] [] -> []
  [V | Vs] [D | Ds] -> [ V : D ; | (premises Vs Ds)])

(define cons-form
  [X | Y] -> [cons (cons-form X) (cons-form Y)]
  X -> X)

(define var
  _ -> (gensym (protect X)))

(defmacro ml-macro
  [ml-datatype Datatype = | ML-Rules] ->
  [datatype Datatype |
            (mapcan (/. ML-Rule
                        (ml->shen Datatype ML-Rule))
                    (parse-ml ML-Rules))])

(define parse-ml
  ML-Rules -> (compile (function <ml-rules>)
                       ML-Rules))
)
