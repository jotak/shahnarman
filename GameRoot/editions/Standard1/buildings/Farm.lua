cost = 4
texture = "Farm"
ethnicity = ""

function getName()
	if language == "french" then
		return "Ferme"
	else
		return "Farm"
	end
end

function getDescription()
	if language == "french" then
		return "+10% de nourriture."
	else
		return "+10% food."
	end
end

function getMod_growth(val)
	return val * 1.1
end
