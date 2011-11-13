
struct Rule {
    virtual ~Rule() =0 {}
    virtual void collect(const TCHAR *query, std::vector<RuleResult> &r) =0;
    virtual Rule *validate() = 0;
    virtual Gdiplus::Bitmap *getIcon(RuleResult *r) { return 0; }

    CString defaultQuery;
};
