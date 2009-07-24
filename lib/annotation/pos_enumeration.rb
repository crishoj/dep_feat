
module Annotation
  class PosEnumeration < Enumeration

    def mark_span(first, last)
      super(first, last)
      # Change POS tags on the commas to that of the conjunction token
      conj_token = last.prev
      @sentence.tokens[first.index .. last.index].each do |tok|
        if tok.form == ','
          tok.cpos = conj_token.cpos
          tok.pos  = conj_token.pos
        end
      end
    end

  end
end
