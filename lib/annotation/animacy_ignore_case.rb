
module Annotation
  class AnimacyIgnoreCase < Animacy

    def pre_corpus
      super
      # Make all lemmas lowercase
      @animate.map! { |lemma| lemma.downcase }
      @dead.map!    { |lemma| lemma.downcase }
    end

    protected
    
    def lookup(token, list)
      # Use lowercase lookup key
      list.include? lookup_key(token).downcase
    end

  end
end
