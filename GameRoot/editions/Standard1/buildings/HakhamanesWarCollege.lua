cost = 15
texture = "a"
ethnicity = ""

function getName()
	if language == "french" then
		return "Coll�ge de guerre"
	else
		return "War college"
	end
end

function getDescription()
	if language == "french" then
		return "..."
	else
		return "..."
	end
end
