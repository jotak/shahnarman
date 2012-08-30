cost = 4
texture = "Barrack"
ethnicity = ""
nbProducedUnits = 2

function getName()
	if language == "french" then
		return "Caserne"
	else
		return "Barrack"
	end
end

function getDescription()
	if language == "french" then
		return "Permet de former des lanciers et épéistes."
	else
		return "Allows to train spearmen and swordmen."
	end
end

function getProducedUnits()
	return "Spearman", 4, "Swordman", 5
end

