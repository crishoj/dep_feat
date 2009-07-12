# To change this template, choose Tools | Templates
# and open the template in the editor.

module Annotation
  class InnerQuotedPhrase < QuotedPhrase

    def mark_span(from, to)
      from = from + 1
      to = to - 1 if to < @sentece.tokens.last.index
      super(from, to)
    end

  end
end
