(datatype rho
  _______________
  [] : process;

  X : process;
  Y : process;
  ===================
  [ X | Y ] : process;

  P : process;
  _________________
  [quote P] : name;

  Act : action;
  Proc : process;
  ===============
  [input Act Process];

  Name : name;
  Process : process;
  =============================
  [lift Name Process] : process;

  Name : name;
  =====================
  [drop Name] : process;

  Proc : process;
  Procs : (list process);
  =============================
  [par Proc | Procs] : process;

  N0 : name;
  N1 : name;
  =====================
  [action N0 N1] : process;

  )
