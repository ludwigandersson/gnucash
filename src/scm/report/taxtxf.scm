;; -*-scheme-*-
;; by  Richard -Gilligan- Uschold 
;;
;; This prints Tax related accounts and exports TXF files for import to
;; TaxCut, TurboTax, etc.
;;
;; For this to work, the user has to segregate taxable and not taxable
;; income to different accounts, as well as deductible and non
;; deductible expenses.
;;
;; The user selects the accounts(s) to be printed, if none, all are checked.
;; Automatically prints up to 15 sub-account levels below selected
;; account.  Accounts below that are not printed. If you really need
;; more levels, change the MAX_LEVELS constant
;;
;; Optionally, does NOT print accounts with $0.00 values.  Prints data
;; between the From and To dates.  Optional alternate periods:
;; "Last Year", "1st Est Tax Quarter", ... "4th Est Tax Quarter"
;; "Last Yr Est Tax Qtr", ... "Last Yr Est Tax Qtr"
;; Estimated Tax Quarters: Dec 31, Mar 31, Jun 30, Aug 31)
;; Optionally prints brief or full account names
;;
;; NOTE: setting of specific dates is squirly! and seems to be
;; current-date dependant!  Actually, time of day dependant!  Just
;; after midnight gives diffenent dates than just before!  Referencing
;; all times to noon seems to fix this.  Subtracting 1 year sometimes
;; subtracts 2!  see "(to-value"

(gnc:support "report/taxtxf.scm")
(gnc:depend "text-export.scm")
(gnc:depend "report-utilities.scm")
(gnc:depend "options.scm")
(gnc:depend "date-utilities.scm")
(gnc:depend "report/txf-export.scm")
(gnc:depend "report/txf-export-help.scm")

(define (make-level-collector num-levels)
  (let ((level-collector (make-vector num-levels)))
    (do ((i 0 (+ i 1)))
        ((= i num-levels) i)
      (vector-set! level-collector i (gnc:make-stats-collector)))
    level-collector))

;; Just a private scope.
(let* ((MAX-LEVELS 16)			; Maximum Account Levels
       (levelx-collector (make-level-collector MAX-LEVELS)))

  ;; This and the next function are the same as in transaction-report.scm
  (define (make-split-list account split-filter-pred)
    (let ((num-splits (gnc:account-get-split-count account)))
      (let loop ((index 0)
                 (split (gnc:account-get-split account 0))
                 (slist '()))
        (if (= index num-splits)
            (reverse slist)
            (loop (+ index 1)
                  (gnc:account-get-split account (+ index 1))
                  (if (split-filter-pred split)
                      (cons split slist)
                      slist))))))

  ;; returns a predicate that returns true only if a split is
  ;; between early-date and late-date
  (define (split-report-make-date-filter-predicate begin-date-tp
                                                   end-date-tp)
    (lambda (split) 
      (let ((tp
             (gnc:transaction-get-date-posted
              (gnc:split-get-parent split))))
        (and (gnc:timepair-ge-date tp begin-date-tp)
             (gnc:timepair-le-date tp end-date-tp)))))

  ;; This is nearly identical to, and could be shared with
  ;; display-report-list-item in report.scm. This adds warn-msg parameter
  (define (gnc:display-report-list-item item port warn-msg)
    (cond
     ((string? item) (display item port))
     ((null? item) #t)
     ((list? item) (map (lambda (item) (gnc:display-report-list-item item port
                                                                     warn-msg))
                        item))
     (else (gnc:warn warn-msg item " is the wrong type."))))

  ;; a few string functions I couldn't find elsewhere
  (define (string-search string sub-str start)
    (do ((sub-len (string-length sub-str))
         ;; must recompute sub-len because order is unknown
         (limit (- (string-length string) (string-length sub-str)))
         (char0 (string-ref sub-str 0))
         ;; find first char of sub-str ; must recompute char0
         (match0 (string-index string (string-ref sub-str 0) start) ; init
                 (string-index string char0 (+ 1 match0))) ; step
         (match #f #f))
        ((or (not match0) (> match0 limit)
             ;; does entire sub-str match?
             (let ()
               (set! match (string=? sub-str (substring string match0 
                                                        (+ match0 sub-len))))
               (if match (set! match match0))
               match))
         match)))

  (define (string-search? string sub-str start)
    (number? (string-search string sub-str start)))

  (define (string-substitute string search-str sub-str start)
    (let ((pos (string-search string search-str start)))
      (if pos
          (let ((search-len (string-length search-str)))
            (string-append (substring string 0 pos) sub-str
                           (substring string (+ pos search-len))))
          string)))

  (define (lx-collector level action value)
    ((vector-ref levelx-collector (- level 1)) action value))

  ;; IRS asked congress to make the tax quarters the same as real quarters
  ;;   This is the year it is effective.  THIS IS A Y10K BUG!
  (define tax-qtr-real-qtr-year 10000)

  (define tax-tab-title (N_ "TAX Report Options"))

  (define (tax-options-generator)
    (options-generator tax-tab-title))

  (define (options-generator tab-title)
    (define gnc:*tax-report-options* (gnc:new-options))
    (define (gnc:register-tax-option new-option)
      (gnc:register-option gnc:*tax-report-options* new-option))

    (gnc:register-tax-option
     (gnc:make-date-option
      tab-title (N_ "From")
      "a" (N_ "Start of reporting period")
      (lambda ()
        (let ((bdtm (gnc:timepair->date (gnc:timepair-canonical-day-time
					 (cons (current-time) 0)))))
	  (set-tm:mday bdtm 1)          ; 01
          (set-tm:mon bdtm 0)           ; Jan
          (set-tm:isdst bdtm -1)
	  (cons 'absolute (cons (car (mktime bdtm)) 0))))
      #f 'absolute #f))

    (gnc:register-tax-option
     (gnc:make-date-option
      tab-title (N_ "To")
      "b" (N_ "End of reporting period")
      (lambda ()
        (cons 'absolute (gnc:timepair-canonical-day-time
			 (cons (current-time) 0))))
      #f 'absolute #f))

    (gnc:register-tax-option
     (gnc:make-multichoice-option
      tab-title (N_ "Alternate Period")
      "c" (N_ "Overide or modify From: & To:") 'from-to
      (list (list->vector
             (list 'from-to (N_ "Use From - To") (N_ "Use From - To period")))
            (list->vector
             (list '1st-est (N_ "1st Est Tax Quarter") (N_ "Jan 1 - Mar 31")))
            (list->vector
             (list '2nd-est (N_ "2nd Est Tax Quarter") (N_ "Apr 1 - May 31")))
            (list->vector
             (list '3rd-est (N_ "3rd Est Tax Quarter") (N_ "Jun 1 - Aug 31")))
            (list->vector
             (list '4th-est (N_ "4th Est Tax Quarter") (N_ "Sep 1 - Dec 31")))
            (list->vector
             (list 'last-year (N_ "Last Year") (N_ "Last Year")))
            (list->vector
             (list '1st-last (N_ "Last Yr 1st Est Tax Qtr")
                   (N_ "Jan 1 - Mar 31, Last year")))
            (list->vector
             (list '2nd-last (N_ "Last Yr 2nd Est Tax Qtr")
                   (N_ "Apr 1 - May 31, Last year")))
            (list->vector
             (list '3rd-last (N_ "Last Yr 3rd Est Tax Qtr")
                   (N_ "Jun 1 - Aug 31, Last year")))
            (list->vector
             (list '4th-last (N_ "Last Yr 4th Est Tax Qtr")
                   (N_ "Sep 1 - Dec 31, Last year"))))))

    (gnc:register-tax-option
     (gnc:make-account-list-option
      tab-title (N_ "Select Accounts (none = all)")
      "d" (N_ "Select accounts")
      (lambda () (gnc:get-current-accounts))
      #f #t))

    (gnc:register-tax-option
     (gnc:make-simple-boolean-option
      tab-title (N_ "Suppress $0.00 values")
      "f" (N_ "$0.00 valued Accounts won't be printed.") #t))

    (gnc:register-tax-option
     (gnc:make-simple-boolean-option
      tab-title (N_ "Print Full account names")
      "g" (N_ "Print all Parent account names") #f))

    (gnc:register-tax-option
     (gnc:make-account-list-option
      (N_ "TXF Export Init") (N_ "Select Account")
      "a" (N_ "Select Account")
      (lambda () (gnc:get-current-accounts))
      #f #t))

    (gnc:register-tax-option
     (gnc:make-simple-boolean-option
      (N_ "TXF Export Init") (N_ "Print extended TXF HELP messages")
      "b" (N_ "Print TXF HELP") #f))

    (gnc:register-tax-option
     ;;(gnc:make-multichoice-option
     (gnc:make-list-option
      (N_ "TXF Export Init")
      (N_ "For INCOME accounts, select here.   < ^ # see help")
      "c" (N_ "Select a TXF Income category")
      '()
      txf-income-categories))

    (gnc:register-tax-option
     ;;(gnc:make-multichoice-option
     (gnc:make-list-option
      (N_ "TXF Export Init")
      (N_ "For EXPENSE accounts, select here.   < ^ # see help")
      "d" (N_ "Select a TXF Expense category")
      '()
      txf-expense-categories))

    (gnc:register-tax-option
     (gnc:make-multichoice-option
      (N_ "TXF Export Init") (N_ "< ^   Payer Name source")
      "e" (N_ "Select the source of the Payer Name") 'default
      (list (list->vector
             (list 'default (N_ "Default")
                   (N_ "Use Indicated Default")))
            (list->vector
             (list 'current (N_ "< Current Account")
                   (N_ "Use Current Account Name")))
            (list->vector
             (list 'parent (N_ "^ Parent Account")
                   (N_ "Use Parent Account Name"))))))

    gnc:*tax-report-options*)

  (define tax-key "{tax}")

  (define tax-end-key "{/tax}")

  ;; Render txf information
  (define txf-last-payer "")		; if same as current, inc txf-l-coount
					; this only works if different
					; codes from the same payer are
					; grouped in the accounts list
  (define txf-l-count 0)		; count repeated N codes
  (define txf-notes "")			; tmp storage for account notes
  (define txf-pos 0)			; tmp storage for tax-end-key in notes
  (define tax-pos 0)			; tmp storage for tax-key in notes
  ;; stores assigned txf codes so we can check for duplicates
  (define txf-dups-alist '())

  (define (txf-payer? str)
    (member str '("<" "^")))

  ;; These gnc:account-get-xxx functions will be relpaced when the tax
  ;; and txf information gets its own account fields, and is no longer
  ;; in the notes field.

  ;; This is a bit of a fudge, matching against strings in account notes.
  ;; It'd be better if these were unique account fields.
  (define (gnc:account-get-tax account)
    (let* ((notes (gnc:account-get-notes account)))
      (string-search? (if notes notes "") tax-key 0)))

  (define (gnc:account-get-txf account)
    (let* ((notes (gnc:account-get-notes account)))
      (set! txf-notes (if notes notes ""))
      (set! txf-pos (string-search txf-notes tax-end-key 0))
      (if txf-pos
	  (begin (set! tax-pos (+ (string-search txf-notes tax-key 0) 
				  (string-length tax-key)))
		 #t)
	  #f)))

  ;; NOTE: You must call gnc:account-get-txf FIRST, or the txf-notes, tax-pos,
  ;;       and txf-pos variables will not be valid!!
  (define (gnc:account-get-txf-code account)
    (if txf-pos
	(substring txf-notes (- txf-pos 4) txf-pos)
	"000"))
  (define (gnc:account-get-txf-format account)
    (if txf-pos
	(string->number (substring txf-notes (- txf-pos 8) (- txf-pos 7)))
	0))
  (define (gnc:account-get-txf-payer-source account)
    (if txf-pos
	(substring txf-notes tax-pos (+ 1 tax-pos))
	" "))
  (define (gnc:account-get-txf-string account)
    (if txf-pos
	(substring txf-notes tax-pos txf-pos)
	" "))

  ;; because we use the list-option input structure, we have to build our own
  ;; search function
  (define (txfq-ref key txf-list)
    (do ((i 0 (+ i 1))
	 (len (length txf-list)))
	((or (>= i len) (eq? key (vector-ref (list-ref txf-list i) 0)))
	 (if (>= i len)
	     (list-ref txf-list 0)
	     (list-ref txf-list i)))))

  ;; return a string to insert in account-notes, or an error symbol
  ;; We only want one, but list-option returns a list.
  (define (txf-string code-lst categories-lst)
    (cond ((or (null? code-lst)
	       (not (symbol? (car code-lst))))
	   'none)
	  ((> (length code-lst) 1)	; only allow ONE selection at a time
	   'mult)			; The GUI should exclude this
	  ((eq? 'N000 (car code-lst))
	   #f)
	  (else
	   (let ((txf-vec (txfq-ref (car code-lst) categories-lst)))
	     (if txf-vec
		 (let ((str (vector-ref txf-vec 1)))
		   (if (equal? "#" (substring str 0 1))
		       'notyet		; not implimented yet
		       (string-append str " \\ "
				      (number->string (vector-ref txf-vec 3))
				      " \\ " (symbol->string 
					      (car code-lst))))))))))

  ;; print txf help strings
  (define (txf-print-help table vect inc)
    (let* ((markup (if inc "income" "expense"))
           (form-desc (vector-ref vect 1))
	   (code (symbol->string (vector-ref vect 0)))
	   (desc-len (string-length form-desc))
	   (bslash (string-search form-desc "\\" 0))
	   (form (substring form-desc 0 (+ bslash 2)))
	   (desc (substring form-desc (+ bslash 2) desc-len))
	   (help (vector-ref vect 2)))
      (gnc:html-table-append-row!
       table
       (list
        (gnc:make-html-table-cell
         (gnc:make-html-text
          (gnc:html-markup markup
                           (gnc:html-markup-b form)
                           code
                           (gnc:html-markup-br)
                           desc)))
         (gnc:make-html-table-cell
          (gnc:make-html-text
           (gnc:html-markup markup help)))))))

  ;; Set or Reset txf string in account notes. str == #f resets.
  ;; Returns a code that indicates the function executed.
  (define (txf-status account key end-key str)
    (let ((key-len (string-length key))
	  (end-len (string-length end-key)))
      (let* ((notes (gnc:account-get-notes account))
	     (notes (if notes notes ""))
	     (key-start (string-search notes key 0))
	     (end-start (string-search notes end-key 0))
	     (notes-len (string-length notes)))

	;; 8 conditions: (key-start, end-start, str) function
	;;                   #f         #f      #f    nothing
	;;                   num        #f      #f    nothing
	;;                   #f         num     #f    nothing (illegal)
	;;                   #f         num     str   nothing (illegal)
	;;                   num        num     #f    reset
	;;                   num        num     str   replace
	;;                   #f         #f      str   set, tax too
	;;                   num        #f      str   set

	(if key-start
	    (let ((key-end (+ key-start key-len)))
	      (cond ((and end-start (not str))
		     ;; reset txf status
		     (let ((ret-val 'remove))
		       (gnc:account-set-notes 
			account (string-append (substring notes 0 key-end)
					       (substring 
						notes (+ end-start end-len)
						notes-len)))
		       ret-val))
		    ((and end-start str)
		     ;; replace txf status with str
		     (let ((ret-val 'replace))
		       (gnc:account-set-notes
			account (string-append (substring notes 0 key-end)
					       str
					       (substring notes end-start
							  notes-len)))
		       ret-val))
		    ((and (not end-start) str)
		     ;; set str and end-key
		     (let ((ret-val 'add))
		       (gnc:account-set-notes
			account (string-append (substring notes 0 key-end)
					       str end-key
					       (substring notes key-end
							  notes-len)))
		       ret-val))
		    (else
		     'none1)))
	    (if (and (not end-start) str)
		;; insert key, str and end-key
		(let ((ret-val 'both))
		  (gnc:account-set-notes account (string-append
						  key str end-key notes))
		  ret-val)
		'none2)))))

  ;; execute the selected function on the account.  Return a list
  ;; containing the function code executed and the txf-string or error message
  (define (txf-function acc txf-inc txf-exp txf-payer)
    (if acc
	(let ((txf-type (gw:enum-<gnc:AccountType>-val->sym
                         (gnc:account-get-type acc) #f)))
	  (if (gnc:account-is-inc-exp? acc)
	      (let* ((str (if (eq? txf-type 'income)
			      (txf-string txf-inc 
					  txf-income-categories)
			      (txf-string txf-exp 
					  txf-expense-categories)))
		     (fun (case str
			    ((mult)
			     (set! str
				   "multiple TXF codes were selected,")
			     'none)
			    ((none)
			     (set! str "no TXF code was selected,")
			     'none)
			    ((notyet)
			     (set! str
				   "selected TXF code is not implimented yet,")
			     'none)
			    (else 
			     (begin 
			       (if (and str (not (eq? txf-payer 'default))
					(txf-payer? (substring str 0 1)))
				   (let ((payer (case txf-payer
						  ((current) "<")
						  ((parent) "^")))
					 (len (string-length str)))
				     (set! str (string-append 
						payer 
						(substring str 1 len)))))
			       (txf-status acc tax-key tax-end-key str))))))
		;; make "<" char html compatable
		(if str
		    (set! str (string-substitute str "<" "&lt;" 0)))
		(list fun str))
	      (list 'notIE "txf-account not of type income or expense")))
	(list 'noAcc "no txf-account")))

  ;; generate a feedback string for the txf function executed
  (define (txf-feedback-str fun-str full-name)
    (case (car fun-str)
      ((none none1 none2 notIE)
       (string-append "No TXF init function"
		      (if (cadr fun-str)
			  (string-append " because, " (cadr fun-str))
			  "")
		      " for account: \"" full-name "\""))
      ((noAcc)
       (string-append "No TXF init function because, " (cadr fun-str)))
      ((remove)
       (string-append "The TXF code was removed from account: \""
		      full-name "\""))
      ((replace)
       (string-append "The TXF code: \"" (cadr fun-str) "\", replaced the "
		      "existing code from account: \"" full-name "\""))
      ((add)
       (string-append "The TXF code: \"" (cadr fun-str)
		      "\", was added to account: \"" full-name "\""))
      ((both)
       (string-append "TAX status was set and the TXF code: \""
		      (cadr fun-str) "\", was added to account: \"" 
		      full-name "\""))))

  ;; check for duplicate txf codes
  (define (txf-check-dups account) 
    (let* ((code (string->symbol (gnc:account-get-txf-code account)))
	   (item (assoc-ref txf-dups-alist code))
	   (payer (gnc:account-get-txf-payer-source account)))
      (if (not (txf-payer? payer))
	  (set! txf-dups-alist (assoc-set! txf-dups-alist code
					   (if item
					       (cons account item)
					       (list account)))))))

  ;; Print error message for duplicate txf codes and accounts
  (define (txf-print-dups doc)
    (let ((dups (apply append
		       (map (lambda (x)
			      (let ((cnt (length (cdr x))))
				(if (> cnt 1)
				    (let* ((acc (cadr x))
					   (txf (gnc:account-get-txf acc)))
				      (cons (string-append 
					     "Code \"" 
					     (gnc:account-get-txf-string acc)
					     "\" has duplicates in "
					     (number->string cnt) " accounts:")
					    (map gnc:account-get-full-name 
						 (cdr x))))
				    '())))
			    txf-dups-alist)))
          (text (gnc:make-html-text)))
      (if (not (null? dups))
	  (begin
            (gnc:html-document-add-object! doc text)
            (gnc:html-text-append!
             text
             (gnc:html-markup-p
              (gnc:html-markup
               "blue"
               (_ "ERROR: There are duplicate TXF codes assigned\
 to some accounts.  Only TXF codes prefixed with \"&lt;\" or \"^\" may be\
 repeated."))))
            (map (lambda (s)
                   (gnc:html-text-append!
                    text
                    (gnc:html-markup-p
                     (gnc:html-markup "blue" s))))
                 dups)))))

  ;; some codes require special handling
  (define (txf-special-split? code)
    (member code '("N521")))            ; only one for now

  (define (render-txf-account account account-value date)
    (let* ((print-info (gnc:account-value-print-info account #f))
	   (value (gnc:amount->string account-value print-info))
	   (txf? (gnc:account-get-txf account)))
      (if (and txf?
	       ;; (not (equal? account-value 0.0)) ; fails, round off, I guess
	       (not (equal? value (gnc:amount->string 0 print-info))))
	  (let* ((type (gw:enum-<gnc:AccountType>-val->sym
                        (gnc:account-get-type account) #f))
		 (code (gnc:account-get-txf-code account))
		 (date-str (if date
			       (strftime "%m/%d/%Y" (localtime (car date)))
			       #f))
		 ;; Only formats 1,3 implimented now! Others are treated as 1.
		 (format (gnc:account-get-txf-format account))
		 (payer-src (gnc:account-get-txf-payer-source account))
		 (account-name (if (equal? payer-src "^")
				   (gnc:account-get-name
				    (gnc:group-get-parent
				     (gnc:account-get-parent account)))
				   (gnc:account-get-name account))) 
		 (value (if (eq? type 'income) ; negate expenses
			    value
			    (string-append 
			     "$-" (substring value 1 (string-length value)))))
		 (l-value (if (txf-payer? payer-src)
			      (begin
				(set! txf-l-count 
				      (if (equal? txf-last-payer account-name)
					  txf-l-count
					  (+ 1 txf-l-count)))
				(set! txf-last-payer account-name)
				(number->string txf-l-count))
			      "1")))
	    (list (if date "\nTD" "\nTS") ; newlines are at the beginning
		  "\n" code		; because one is added at the end, and
		  "\nC1"		; TurboTax spits out an error msg for
		  "\nL" l-value		; a blank line at the end of the file.
		  (if date
		      (list "\nD" date-str)
		      '())
		  "\n" value
		  (case format
		    ((3) (list "\nP" account-name))
		    (else '()))
		  "\n^"))
	  "")))

  ;; Render any level
  (define (render-level-x-account table level max-level account lx-value
				  suppress-0 full-names txf-date)
    (let* ((account-name (if txf-date	; special split
			     (strftime "%Y-%b-%d" (localtime (car txf-date)))
			     (if (or full-names (equal? level 1))
				 (gnc:account-get-full-name account)
				 (gnc:account-get-name account))))
	   (blue? (gnc:account-get-txf account))
	   (print-info (gnc:account-value-print-info account #f))
	   (value (gnc:amount->string lx-value print-info))
	   (value-formatted
            (gnc:make-html-text
             (if blue?
                 (gnc:html-markup "blue" value)
                 value)))
	   (account-name (if blue?
                             (gnc:html-markup "blue" account-name)
                             account-name))
           (blank-cells (make-list (- max-level level)
                                   (gnc:make-html-table-cell #f))))
      (if (and blue? (not txf-date))	; check for duplicate txf codes
	  (txf-check-dups account))
      ;;(if (not (equal? lx-value 0.0)) ; this fails, round off, I guess
      (if (or (not suppress-0) (= level 1)
              (not (equal? value (gnc:amount->string 0 print-info))))
          (gnc:html-table-append-row!
           table
           (append
            (list
             (gnc:make-html-table-cell
              (apply gnc:make-html-text
                     (append (make-list level "&nbsp;")
                             (list account-name)))))
            blank-cells
            (list
             (gnc:make-html-table-cell/markup "number-cell"
                                              value-formatted)))))))

  ;; Recursivly validate children if parent is not a tax account.
  ;; Don't check children if parent is valid.
  ;; Returns the Parent if a child or grandchild is valid.
  (define (validate accounts)
    (apply append (map (lambda (a)
                         (if (gnc:account-get-tax a)
                             (list a)
                             ;; check children
                             (if (null? (validate
                                         (gnc:group-get-subaccounts
                                          (gnc:account-get-children a))))
                                 '()
                                 (list a))))
                       accounts)))

  (define (generate-tax-or-txf report-name
			       report-description
			       report-obj
			       tax-mode-in)

    (define (get-option pagename optname)
      (gnc:option-value
       (gnc:lookup-option 
        (gnc:report-options report-obj) pagename optname)))

    ;; the number of account generations: children, grandchildren etc.
    (define (num-generations account gen)
      (let ((children (gnc:account-get-children account)))
	(if (not children)
	    (if (and (gnc:account-get-txf account)
		     (equal? "N521" (gnc:account-get-txf-code account)))
		(+ gen 1)		; Est Fed Tax has a extra generation
		gen)	       		; no kids, return input
	    (apply max (gnc:group-map-accounts
			(lambda (x) (num-generations x (+ 1 gen)))
			children)))))

    (let* ((tab-title tax-tab-title)
	   (from-value (gnc:date-option-absolute-time 
			(get-option tab-title "From")))
           (to-value (gnc:timepair-end-day-time
		      (gnc:date-option-absolute-time 		       
		       (get-option tab-title "To"))))
	   (alt-period (get-option tab-title "Alternate Period"))
	   (suppress-0 (get-option tab-title "Suppress $0.00 values"))
	   (full-names (get-option tab-title
                                   "Print Full account names"))
	   (tax-mode tax-mode-in)	; these need to different later
	   (user-sel-accnts (get-option tab-title
                                        "Select Accounts (none = all)"))
	   (valid-user-sel-accnts (validate user-sel-accnts))
	   ;; If no selected accounts, check all.
	   (selected-accounts (if (not (null? user-sel-accnts))
				  valid-user-sel-accnts
				  (validate (gnc:group-get-subaccounts
					     (gnc:get-current-group)))))
	   (generations (if (pair? selected-accounts)
			    (apply max (map (lambda (x) (num-generations x 1))
					    selected-accounts))
			    0))
	   (max-level (min MAX-LEVELS (max 1 generations)))

	   ;; Alternate dates are relative to from-date
	   (from-date (gnc:timepair->date from-value))
	   (from-value (gnc:timepair-start-day-time
			(let ((bdtm from-date))
			  (if (member alt-period 
				      '(last-year 1st-last 2nd-last
						  3rd-last 4th-last))
			      (set-tm:year bdtm (- (tm:year bdtm) 1)))
			  (set-tm:mday bdtm 1)
			  (if (< (gnc:date-get-year bdtm) 
				 tax-qtr-real-qtr-year)
			      (case alt-period
				((1st-est 1st-last last-year) ; Jan 1
				 (set-tm:mon bdtm 0))
				((2nd-est 2nd-last) ; Apr 1
				 (set-tm:mon bdtm 3))
				((3rd-est 3rd-last) ; Jun 1
				 (set-tm:mon bdtm 5))
				((4th-est 4th-last) ; Sep 1
				 (set-tm:mon bdtm 8)))
			      ;; Tax quaters equal Real quarters
			      (case alt-period
				((1st-est 1st-last last-year) ; Jan 1
				 (set-tm:mon bdtm 0))
				((2nd-est 2nd-last) ; Apr 1
				 (set-tm:mon bdtm 3))
				((3rd-est 3rd-last) ; Jul 1
				 (set-tm:mon bdtm 6))
				((4th-est 4th-last) ; Oct 1
				 (set-tm:mon bdtm 9))))
                          (set-tm:isdst bdtm -1)
			  (cons (car (mktime bdtm)) 0))))

	   (to-value (gnc:timepair-end-day-time
		      (let ((bdtm from-date))
			(if (member alt-period 
				    '(last-year 1st-last 2nd-last
						3rd-last 4th-last))
			    (set-tm:year bdtm (- (tm:year bdtm) 1)))
			;; Bug! Above subtracts two years, should only be one!
			;; The exact same code, in from-value, further above,
			;;   only subtraces one!  Go figure!
			;; So, we add one back below!
			(if (member alt-period 
				    '(last-year 1st-last 2nd-last
						3rd-last 4th-last))
			    (set-tm:year bdtm (+ (tm:year bdtm) 1)))
			(set-tm:mday bdtm 31)
			(if (< (gnc:date-get-year bdtm) tax-qtr-real-qtr-year)
			    (case alt-period
			      ((1st-est 1st-last) ; Mar 31
			       (set-tm:mon bdtm 2))
			      ((2nd-est 2nd-last) ; May 31
			       (set-tm:mon bdtm 4))
			      ((3rd-est 3rd-last) ; Aug 31
			       (set-tm:mon bdtm 7))
			      ((4th-est 4th-last last-year) ; Dec 31
			       (set-tm:mon bdtm 11))
			      (else (set! bdtm (gnc:timepair->date to-value))))
			    ;; Tax quaters equal Real quarters
			    (case alt-period
			      ((1st-est 1st-last) ; Mar 31
			       (set-tm:mon bdtm 2))
			      ((2nd-est 2nd-last) ; Jun 30
			       (set-tm:mday bdtm 30)
			       (set-tm:mon bdtm 5))
			      ((3rd-est 3rd-last) ; Sep 30
			       (set-tm:mday bdtm 30)
			       (set-tm:mon bdtm 8))
			      ((4th-est 4th-last last-year) ; Dec 31
			       (set-tm:mon bdtm 11))
			      (else 
			       (set! bdtm (gnc:timepair->date to-value)))))
                        (set-tm:isdst bdtm -1)
			(cons (car (mktime bdtm)) 0))))

           (txf-help (get-option "TXF Export Init"
                                 "Print extended TXF HELP messages"))
	   (txf-feedback-str-lst '())
           (doc (gnc:make-html-document))
           (table (gnc:make-html-table)))

      (define (handle-txf-special-splits level account from-value to-value)
	(if (and (gnc:account-get-txf account)
		 (txf-special-split? (gnc:account-get-txf-code account)))
	    (let* 
		((full-year?
                  (let ((bdto (localtime (car to-value)))
                        (bdfrom (localtime (car from-value))))
                    (and (equal? (tm:year bdto) (tm:year bdfrom))
                         (equal? (tm:mon bdfrom) 0)
                         (equal? (tm:mday bdfrom) 1)
                         (equal? (tm:mon bdto) 11)
                         (equal? (tm:mday bdto) 31))))
		 ;; Adjust dates so we get the final Estimated Tax
		 ;; paymnent from the right year
		 (from-est (if full-year?
			       (let ((bdtm (gnc:timepair->date
					    (gnc:timepair-canonical-day-time
					     from-value))))
				 (set-tm:mday bdtm 1) ; 01
				 (set-tm:mon bdtm 2) ; Mar
                                 (set-tm:isdst bdtm -1)
				 (cons (car (mktime bdtm)) 0))
			       from-value))
		 (to-est (if full-year?
			     (let* ((bdtm (gnc:timepair->date
					   (gnc:timepair-canonical-day-time
					    from-value))))
			       (set-tm:mday bdtm 28) ; 28
			       (set-tm:mon bdtm 1) ; Feb
			       (set-tm:year bdtm (+ (tm:year bdtm) 1))
                               (set-tm:isdst bdtm -1)
                               (cons (car (mktime bdtm)) 0))
			     to-value))
		 (split-filter-pred (split-report-make-date-filter-predicate
				     from-est to-est))
		 (split-list (make-split-list account split-filter-pred))
		 (lev  (if (>= max-level (+ 1 level))
			   (+ 1 level)
			   level)))
	      (map (lambda (spl) 
		     (let* ((tmp-date (gnc:transaction-get-date-posted 
				       (gnc:split-get-parent spl)))
			    (value (gnc:numeric-to-double
                                    (gnc:split-get-value spl)))
                            ;; TurboTax '99 ignores dates after 12/31/1999
                            (date (if (and full-year? 
                                           (gnc:timepair-lt to-value tmp-date))
                                      to-value
                                      tmp-date)))
		       (if tax-mode
			   (render-level-x-account table lev max-level account
						   value suppress-0 #f date #f)
			   (render-txf-account account value date))))
		   split-list))
	    '()))

      (define (handle-level-x-account level account)
	(let ((type (gw:enum-<gnc:AccountType>-val->sym
                     (gnc:account-get-type account) #f))
	      (name (gnc:account-get-name account)))

	  (if (gnc:account-is-inc-exp? account)
	      (let ((children (gnc:account-get-children account))
                    (account-balance (if (gnc:account-get-tax account)
                                         (gnc:account-get-balance-interval
                                          account from-value to-value #f)
                                         0))) ; don't add non tax related

                (if (and tax-mode (not children))
                    (handle-txf-special-splits 
                     level account from-value to-value))

                (set! account-balance (+ (if (> max-level level)
					     (lx-collector (+ 1 level)
							   'total #f)
					     0)
					 ;; make positive
					 (if (eq? type 'income)
					     (- account-balance)
					     account-balance)))
		(lx-collector level 'add account-balance)

		(let ((level-x-output
		       (if tax-mode
			   (render-level-x-account table level
                                                   max-level account
						   account-balance
						   suppress-0 full-names #f)
			   (render-txf-account account account-balance #f))))
		  (if (equal? 1 level)
		      (lx-collector 1 'reset #f))
		  (if (> max-level level)
		      (lx-collector (+ 1 level) 'reset #f))

                  (if (and tax-mode children)
                      (gnc:group-map-accounts
                       (lambda (x)
                         (if (>= max-level (+ 1 level))
                             (handle-level-x-account (+ 1 level) x)))
                       children)))))))

      (let* ((txf-acc-lst (get-option "TXF Export Init" "Select Account"))
             (txf-account (if (null? txf-acc-lst)
                              (begin (set! txf-acc-lst '(#f))
                                     #f)
                              (car txf-acc-lst)))
             (txf-income (get-option "TXF Export Init"
                                     "For INCOME accounts,\
 select here.   < ^ # see help"))
             (txf-expense (get-option "TXF Export Init"
                                      "For EXPENSE\
 accounts, select here.   < ^ # see help"))
             (txf-payer-source (get-option "TXF Export Init"
                                           "< ^   Payer Name source"))
             (txf-full-name-lst (if txf-account
                                    (map gnc:account-get-full-name
                                         txf-acc-lst)
                                    '(#f)))

             (txf-fun-str-lst (map (lambda (a) (txf-function 
                                                a txf-income txf-expense
                                                txf-payer-source))
                                   txf-acc-lst)))
        (set! txf-feedback-str-lst (map txf-feedback-str txf-fun-str-lst
                                        txf-full-name-lst)))

      (let ((from-date  (strftime "%Y-%b-%d" (localtime (car from-value))))
	    (to-date    (strftime "%Y-%b-%d" (localtime (car to-value))))
	    (today-date (strftime "D%m/%d/%Y" 
				  (localtime 
				   (car (gnc:timepair-canonical-day-time
					 (cons (current-time) 0))))))
	    (txf-last-payer "")
	    (txf-l-count 0)
	    (report-title (if txf-help 
                              (_ "Detailed TXF Category Descriptions")
			      report-name))
	    (file-name #f))

        (define (make-sub-headers max-level)
          (if (eq? max-level 0)
              '()
              (cons (gnc:make-html-table-header-cell/markup
                     "number-header"
                     "(" (_ "Sub") " " (number->string max-level) ")")
                    (make-sub-headers (- max-level 1)))))

	;; Now, the main body
	;; Reset all the balance collectors
	(do ((i 1 (+ i 1)))
	    ((> i MAX-LEVELS) i)
	  (lx-collector i 'reset #f))
	(set! txf-dups-alist '())

	(if (not tax-mode-in)		; First do Txf mode, if set
	    (begin
	      (set! file-name		; get file name from user
		    (do ((fname (gnc:file-selection-dialog
				 "Select file for .TXF export" ""
				 "~/export.txf")
				(gnc:file-selection-dialog
				 "Select file for .TXF export" ""
				 "~/export.txf")))  
			((if (not fname)
			     #t		; no "Cancel" button, exit
			     (if (access? fname F_OK)
				 (if (gnc:verify-dialog
				      (string-append 
				       "File: \"" fname
				       "\" exists, Overwrite?")
				      #f)
				     (begin (delete-file fname)
					    #t)
				     #f)
				 #t))
			 fname)))

	      (if file-name		; cancel TXF if no file selected
		  (let* ((port (open-output-file file-name))    
			 (output
                          (list
                           (map (lambda (x) (handle-level-x-account 1 x))
                                selected-accounts)))
			 (output-txf (list
				      "V035"
				      "\nAGnuCash "
                                      gnc:version
				      "\n" today-date
				      "\n^"
				      output)))

		    (gnc:display-report-list-item output-txf port
						  "taxtxf.scm - ")
		    (newline port)
		    (close-output-port port)))))

	(set! tax-mode #t)		; now do tax mode to display report

        (gnc:html-document-set-style! 
         doc "blue"
         'tag "font"
         'attribute (list "color" "#0000ff"))

        (gnc:html-document-set-style! 
         doc "income"
         'tag "font"
         'attribute (list "color" "#0000ff"))

        (gnc:html-document-set-style! 
         doc "expense"
         'tag "font"
         'attribute (list "color" "#ff0000"))

        (gnc:html-document-set-title! doc report-title)

        (gnc:html-document-add-object! 
         doc
         (gnc:make-html-text         
          (gnc:html-markup-p
           (gnc:html-markup/format
            (_ "Period from %s to %s") from-date to-date))))

        (gnc:html-document-add-object!
         doc
         (gnc:make-html-text
          (gnc:html-markup
           "blue"
           (if tax-mode-in
               (if (not txf-help)
                   (gnc:html-markup-p
                    (_ "Blue items are exportable to a TXF file."))
                   "")
               (gnc:html-markup-p
                (if file-name
                    (gnc:html-markup/format
                     (_ "Blue items were exported to file %s.")
                     (gnc:html-markup-tt file-name))
                    (_ "Blue items were <em>not</em> exported to \
txf file!")))))))

        (if (not txf-help)
            (map (lambda (s)
                   (gnc:html-document-add-object!
                    doc
                    (gnc:make-html-text
                     (gnc:html-markup "blue" (gnc:html-markup-p s)))))
                 txf-feedback-str-lst))

        (txf-print-dups doc)

        (gnc:html-document-add-object! doc table)

        (if txf-help
            (gnc:html-document-set-style! doc
                                          "table"
                                          'attribute (list "border" "1")))

        (if txf-help
            (gnc:html-table-append-row!
             table
             (list
              (gnc:make-html-table-header-cell
               (_ "Tax Form \\ TXF Code")
               (gnc:make-html-text (gnc:html-markup-br))
               (_ "Description"))
              (gnc:make-html-table-header-cell
               (_ "Extended TXF Help messages") " "
               (gnc:make-html-text
                (gnc:html-markup "income" (_ "Income")))
               " "
               (gnc:make-html-text
                (gnc:html-markup "expense" (_ "Expense"))))))
            (gnc:html-table-append-row!
             table
             (append
              (list
               (gnc:make-html-table-header-cell
                (_ "Account Name")))
              (make-sub-headers max-level)
              (list
               (gnc:make-html-table-header-cell/markup
                "number-header" (_ "Total"))))))

        (if txf-help
            (begin
              (map (lambda (x) (txf-print-help table x #t))
                   txf-help-categories)
              (map (lambda (x) (txf-print-help table x #t))
                   txf-income-categories)
              (map (lambda (x) (txf-print-help table x #f))
                   txf-expense-categories))
            (map (lambda (x) (handle-level-x-account 1 x))
                 selected-accounts))

        (if (null? selected-accounts)
            (gnc:html-document-add-object!
             doc
             (gnc:make-html-text
              (gnc:html-markup-p
               (_ "No Tax Related accounts were found. \
Go the the Tax Information dialog to set up tax-related accounts.")))))

        doc)))

;  (gnc:define-report
;   'version 1
;   'name (N_ "Tax")
;   'options-generator tax-options-generator
;   'renderer (lambda (report-obj)
;               (generate-tax-or-txf
;                (_ "Taxable Income / Deductible Expenses")
;                (_ "This page shows your Taxable Income and \
;Deductable Expenses.")
;                report-obj
;		#t)))
  #f)

;  (gnc:define-report
;   'version 1
;   'name (N_ "Export .TXF")
;   'options-generator tax-options-generator
;   'renderer (lambda (report-obj)
;               (generate-tax-or-txf
;                (_ "Taxable Income / Deductible Expenses")
;                (_ "This page shows your Taxable Income and \
;Deductable Expenses.")
;                report-obj
;		#f))))
