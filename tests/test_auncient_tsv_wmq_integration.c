^{:wmq-job "INTJOB"
  :wmq-compiler "FOLKLORE"
  :wmq-mount "STANAG"
  :wmq-author "SYSTEM"
  :wmq-params {:tin 950000000 :ssn 050051122}}

(defn -main []
  (wmq-reg-write 1 10)
  (wmq-reg-write 2 10)
  (wmq-hathitrust-lookup)
  (wmq-reg-read 5)
  
  (wmq-connect-idx 2)
  (wmq-poke 0 78125)
  (wmq-peek-idx 0)
  
  (wmq-reg-write 1 201308)
  (wmq-reg-write 4 1)
  (wmq-reg-read 5)
  
  (wmq-keycodes 32 30)
  (wmq-disconnect)
  (exit 899025))
