(gnc:support "report/folio.scm")
(gnc:depend "report-utilities.scm")
(gnc:depend "html-generator.scm")

(let ()

  (define (folio-options-generator)

    (define gnc:*folio-report-options* (gnc:new-options))
    (define (gnc:register-folio-option new-option)
      (gnc:register-option gnc:*folio-report-options* new-option)) 

    (gnc:register-folio-option
     (gnc:make-date-option
      (N_ "Portfolio Options") (N_ "At")
      "a" (N_ "Calculate stock portfolio value at this date")
      (lambda ()
        (let ((bdtime (localtime (current-time))))
          (set-tm:sec bdtime 59)
          (set-tm:min bdtime 59)
          (set-tm:hour bdtime 23)
          (let ((time (car (mktime bdtime))))
            (cons 'absolute (cons time 0)))))
      #f 'absolute #f))

    gnc:*folio-report-options*)

  (define (titles)
    (list (_ "Name") (_ "Ticker") (_ "Shares") (_ "Recent Price")
          (_ "Value") (_ "Cost") (_ "Profit/Loss")))

  (define (gnc:account-get-last-split account)
    (let ((num-splits (gnc:account-get-split-count account)))
      (gnc:account-get-split account (if (> num-splits 0)
                                         (- num-splits 1)
                                         0))))

  (define (report-rows)

    (define total-value (make-stats-collector))
    (define total-cost (make-stats-collector))

    (define blank-row
      (list "&nbsp" "&nbsp" "&nbsp" "&nbsp" "&nbsp" "&nbsp" "&nbsp"))

    (define (report-row account)
      (let ((last-split (gnc:account-get-last-split account)))
        (let ((shares (d-gnc:split-get-share-balance last-split))
              (price (d-gnc:split-get-share-price last-split))
              (balance (d-gnc:split-get-balance last-split))
              (cost 0) ; fixme (gnc:split-get-cost-basis last-split)))
              (quantity-print-info
               (gnc:split-quantity-print-info last-split #f))
              (price-print-info (gnc:default-price-print-info))
              (value-print-info
               (gnc:split-value-print-info last-split #f)))

          (total-value 'add balance)
          (total-cost 'add cost)

          (list
           (gnc:account-get-name account)
           (gnc:commodity-get-printname 
            (gnc:account-get-security account))
           (gnc:amount->string shares quantity-print-info)
           (gnc:amount->string price price-print-info)
           (gnc:amount->string balance value-print-info)
           (gnc:amount->string cost value-print-info)
           (gnc:amount->string (- balance cost) value-print-info)))))

    (define (net-row)
      (let ((value (total-value 'total #f))
            (cost (total-cost 'total #f))
            (print-info (gnc:default-print-info #f)))
        (list (html-strong (_ "Net"))
              "&nbsp" "&nbsp" "&nbsp"
              (gnc:amount->string value print-info)
              (gnc:amount->string cost print-info)
              (gnc:amount->string (- value cost) print-info))))

    (define (report-rows-main)
      (gnc:group-map-all-accounts
       (lambda (account)
         (let ((type (gw:enum-GNCAccountType-val->sym
                      (gnc:account-get-type account)
                      #f)))
           (if (member type '(stock mutual-fund))
               (report-row account)
               #f)))
       (gnc:get-current-group)))

    (define (collapse list collapsed)
      (cond ((null? list) collapsed)
            (else (collapse (cdr list)
                            (if (car list)
                                (cons (car list) collapsed)
                                collapsed)))))

    (let ((main-rows (collapse (report-rows-main) '())))
      (reverse (cons (net-row)
                     (cons blank-row main-rows)))))

  (define (folio-renderer options)
    (list
     (html-start-document-title (_ "Stock Portfolio Valuation") "#bfdeba")
     (html-table (_ "Stock Portfolio Valuation") (titles) (report-rows))
     (html-end-document)))

  (gnc:define-report
   'version 1
   'name (N_ "Stock Portfolio")
   'renderer folio-renderer))
