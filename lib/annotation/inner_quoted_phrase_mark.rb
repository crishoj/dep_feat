require 'lib/annotation/inner_quoted_phrase'

module Annotation
  class InnerQuotedPhraseMark < InnerQuotedPhrase

    def mark_span(from, to)
      from.pos = 'XQT' if OPENINGS.include? from.form
      to.pos   = 'XQT' if CLOSINGS.include? to.form
      super(from, to)
    end

  end
end
