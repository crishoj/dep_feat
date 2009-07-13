
module Annotation
  class InnerQuotedPhrase < QuotedPhrase

    def mark_span(from, to)
      from = from.next
      to   = to.prev if to.index < @sentence.tokens.last.index
      super(from, to)
    end

  end
end
