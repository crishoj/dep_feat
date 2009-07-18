
module Annotation
  class Enumeration < Base

    def pre_sentence
      @sentence.grep(:cpos => 'C') do |tok|
        if tok.next.cpos == 'N'
          if leading_comma(tok)
            # Seems to be an enumeration
            first = first_comma(tok)
            @sentence.tokens[first.index .. tok.next.index].each do |t|
              puts "marking #{t.form}"
              t.features << 'enum'
            end
          end
        end
      end
    end

    protected

    def leading_comma(token)
      token.leading(3).find { |t| t.form == ',' }
    end

    def first_comma(token)
      while (token = leading_comma(token))
        comma = token
      end
      comma
    end

  end
end
