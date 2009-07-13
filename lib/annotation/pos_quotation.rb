
require 'lib/annotation/quotation'

module Annotation

  # As Quotation, but puts introduces special POS tags
  # instead of relying on features.
  class PosQuotation < Quotation

    def prefix
      "Q"
    end

    protected

    def mark_span(first, last)
      last = (@sentence.tokens.size-1) if last == :end
      for token in @sentence.tokens[first..last]
        token.pos  = "#{prefix}-#{token.pos}"
      end
    end

    def sentence_affected?
      @sentence.tokens.find { |tok| tok.pos =~ /^Q-/ }
    end

  end
end
