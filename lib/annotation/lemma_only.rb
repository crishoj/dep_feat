
module Annotation
  class LemmaOnly < Lemma

    def mark_token
      super
      @token.form = @token.lemma
      @token.lemma = nil
    end

  end
end