    .export _fujinet_logo
    .export _fujinet_logo_pal

    .data
_fujinet_logo:
    .incbin "./logo/fujinet_logo.spr"

_fujinet_logo_pal:

    .byte $0F,$0E,$0A,$09,$0B,$06,$0C,$0D,$03,$00,$01,$04,$08,$05,$07,$02
	.byte $FF,$EE,$AA,$99,$BB,$66,$CC,$DD,$33,$00,$11,$44,$88,$55,$77,$22


; fujinet_logo_redir:	dc.b $01,$23,$45,$67,$89,$AB,$CD,$EF
;fujinet_logo_pal:	dc.b $0F,$0E,$0A,$09,$0B,$06,$0C,$0D,$03,$00,$01,$04,$08,$05,$07,$02
;	                dc.b $FF,$EE,$AA,$99,$BB,$66,$CC,$DD,$33,$00,$11,$44,$88,$55,$77,$22
