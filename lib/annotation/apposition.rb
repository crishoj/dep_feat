
module Annotation
  class Apposition < Span

    def markers
      %w{, -}
    end

    def feature
      'appos'
    end

    def mark_token
      for marker in markers
        if @token.form == marker
          @from = @token
          if @to = @from.trailing(10).find { |t| t.form == marker }
            contents = @sentence[@from.index+1 .. @to.index-1]
            if contents.all? { |tok| valid_appositional? tok }
              mark_span(@from, @to)
            end
          end
        end
      end
    end

    def valid_appositional? token
      not %w{" ( )}.include? token.form
    end

  end
end
